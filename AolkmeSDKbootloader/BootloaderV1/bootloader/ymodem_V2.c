/**
  ******************************************************************************
  * @file    ymodem.c 
  * @author  MCD Application Team
  * @brief   YModem protocol implementation
  * @version V2.0.0 - 重构版本
  ******************************************************************************
  * @attention
  * 重构改进点：
  * 1. 使用状态机管理传输过程
  * 2. 精确的超时处理（基于系统滴答）
  * 3. 分离协议层和硬件操作层
  * 4. 增强错误处理和调试信息
  * 5. 保持原有函数接口兼容性
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ymodem.h"
#include "flash_if.h"
#include "common.h"
#include "string.h"

/* Private typedef -----------------------------------------------------------*/
/* 重构新增：内部状态结构 */
typedef struct {
    uint8_t header;
    uint8_t packet_num;
    uint8_t packet_num_complement;
    uint16_t data_length;
    uint16_t crc_value;
} Packet_InfoTypeDef;

/* Private define ------------------------------------------------------------*/
/* 重构新增：内部常量 */
#define PACKET_VALID            (0x01)
#define PACKET_INVALID          (0x00)

/* Private macro -------------------------------------------------------------*/
/* 重构新增：安全检查宏 */
#define IS_VALID_PACKET_NUM(pkt_num, comp_num)  (((uint8_t)((pkt_num) + (comp_num))) == 0xFF)

/* Private variables ---------------------------------------------------------*/
/* 保持原有全局变量 */
uint8_t aFileName[FILE_NAME_LENGTH];
__IO uint32_t flashdestination = APPLICATION_ADDRESS;

/* 重构改进：缓冲区对齐优化，避免内存访问问题 */
__ALIGN_BEGIN uint8_t aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE] __ALIGN_END;

/* 重构新增：会话上下文 */
static YModem_SessionTypeDef ymodem_session;

/* Private function prototypes -----------------------------------------------*/
/* 重构改进：内部函数声明 */
static void PrepareIntialPacket(uint8_t *p_data, const uint8_t *p_file_name, uint32_t length);
static void PreparePacket(uint8_t *p_source, uint8_t *p_packet, uint8_t pkt_nr, uint32_t size_blk);
static COM_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout_ms);
static COM_StatusTypeDef ValidatePacket(const uint8_t *p_data, uint32_t packet_length);
static uint32_t ParseFileInfo(const uint8_t *p_data, uint32_t packet_length);
static COM_StatusTypeDef ProcessDataPacket(uint32_t packet_length);
static COM_StatusTypeDef WaitForResponse(uint8_t expected_response, uint32_t timeout_ms);
static void ResetSession(void);
static uint32_t GetElapsedTime(uint32_t start_time);

/* 重构新增：硬件抽象函数声明 - 需要用户实现 */
static HAL_StatusTypeDef Uart_ReadWithTimeOut(uint8_t *data, uint16_t len, uint32_t timeOut_ms);
static HAL_StatusTypeDef Uart_WriteWithTimeOut(uint8_t *data, uint16_t len, uint32_t timeOut_ms);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  重置YModem会话
  * @param  None
  * @retval None
  * @note   重构新增：集中管理会话状态重置
  */
static void ResetSession(void)
{
    ymodem_session.state = YMODEM_STATE_IDLE;
    ymodem_session.file_size = 0;
    ymodem_session.bytes_received = 0;
    ymodem_session.write_address = APPLICATION_ADDRESS;
    ymodem_session.packet_number = 0;
    ymodem_session.error_count = 0;
    ymodem_session.session_start_time = Get_TickCount();
    memset(ymodem_session.filename, 0, sizeof(ymodem_session.filename));
}

/**
  * @brief  计算经过的时间
  * @param  start_time: 开始时间
  * @retval 经过的毫秒数
  * @note   重构改进：防止时间回绕
  */
static uint32_t GetElapsedTime(uint32_t start_time)
{
    uint32_t current_time = Get_TickCount();
    if (current_time >= start_time) {
        return current_time - start_time;
    } else {
        /* 处理计数器回绕 */
        return (0xFFFFFFFF - start_time) + current_time;
    }
}

/**
  * @brief  验证数据包完整性
  * @param  p_data: 数据包指针
  * @param  packet_length: 数据包长度
  * @retval COM_StatusTypeDef 验证结果
  * @note   重构新增：集中数据包验证逻辑
  */
static COM_StatusTypeDef ValidatePacket(const uint8_t *p_data, uint32_t packet_length)
{
    uint16_t calculated_crc, received_crc;
    
    /* 检查包序号连续性 */
    if (!IS_VALID_PACKET_NUM(p_data[PACKET_NUMBER_INDEX], p_data[PACKET_CNUMBER_INDEX])) {
        YMODEM_DEBUG("Packet sequence error: %02X vs %02X", 
                    p_data[PACKET_NUMBER_INDEX], 
                    (uint8_t)~p_data[PACKET_CNUMBER_INDEX]);
        return COM_SEQUENCE_ERROR;
    }
    
    /* CRC校验 */
    received_crc = (uint16_t)(p_data[packet_length + PACKET_DATA_INDEX] << 8);
    received_crc += p_data[packet_length + PACKET_DATA_INDEX + 1];
    calculated_crc = Cal_CRC16(&p_data[PACKET_DATA_INDEX], packet_length);
    
    if (calculated_crc != received_crc) {
        YMODEM_DEBUG("CRC error: expected 0x%04X, got 0x%04X", 
                    calculated_crc, received_crc);
        return COM_CRC_ERROR;
    }
    
    return COM_OK;
}

/**
  * @brief  解析文件头信息
  * @param  p_data: 数据包指针
  * @param  packet_length: 数据包长度
  * @retval 文件大小，0表示解析失败
  * @note   重构改进：更健壮的文件信息解析
  */
static uint32_t ParseFileInfo(const uint8_t *p_data, uint32_t packet_length)
{
    uint32_t i = 0, j = 0;
    uint32_t file_size = 0;
    uint8_t file_size_str[FILE_SIZE_LENGTH] = {0};
    
    /* 解析文件名 */
    while (i < packet_length && p_data[PACKET_DATA_INDEX + i] != 0 && 
           j < FILE_NAME_LENGTH - 1) {
        ymodem_session.filename[j++] = p_data[PACKET_DATA_INDEX + i++];
    }
    ymodem_session.filename[j] = '\0';
    
    YMODEM_DEBUG("Receiving file: %s", ymodem_session.filename);
    
    /* 跳过文件名结束的NULL字符 */
    if (i < packet_length && p_data[PACKET_DATA_INDEX + i] == 0) {
        i++;
    }
    
    /* 解析文件大小 */
    j = 0;
    while (i < packet_length && p_data[PACKET_DATA_INDEX + i] != ' ' && 
           p_data[PACKET_DATA_INDEX + i] != 0 && j < FILE_SIZE_LENGTH - 1) {
        file_size_str[j++] = p_data[PACKET_DATA_INDEX + i++];
    }
    file_size_str[j] = '\0';
    
    /* 转换文件大小字符串为整数 */
    if (file_size_str[0] != '\0') {
        Str2Int(file_size_str, &file_size);
        YMODEM_DEBUG("File size: %lu bytes", file_size);
    }
    
    return file_size;
}

/**
  * @brief  处理数据包内容
  * @param  packet_length: 数据包长度
  * @retval COM_StatusTypeDef 处理结果
  * @note   重构改进：分离数据处理和Flash操作
  */
static COM_StatusTypeDef ProcessDataPacket(uint32_t packet_length)
{
    uint32_t ramsource;
    uint16_t bytes_to_write;
    
    /* 计算实际需要写入的数据长度（最后一包可能不满） */
    bytes_to_write = packet_length;
    if (ymodem_session.bytes_received + packet_length > ymodem_session.file_size) {
        bytes_to_write = ymodem_session.file_size - ymodem_session.bytes_received;
    }
    
    /* 写入Flash */
    ramsource = (uint32_t)&aPacketData[PACKET_DATA_INDEX];
    if (FLASH_If_Write(ymodem_session.write_address, (uint8_t *)ramsource, bytes_to_write) != FLASHIF_OK) {
        YMODEM_DEBUG("Flash write error at address 0x%08lX", ymodem_session.write_address);
        return COM_FLASH_ERROR;
    }
    
    /* 更新进度 */
    ymodem_session.write_address += bytes_to_write;
    ymodem_session.bytes_received += bytes_to_write;
    ymodem_session.packet_number++;
    
    YMODEM_DEBUG("Progress: %lu/%lu bytes (%.1f%%)", 
                ymodem_session.bytes_received, 
                ymodem_session.file_size,
                (ymodem_session.bytes_received * 100.0f) / ymodem_session.file_size);
    
    return COM_OK;
}

/**
  * @brief  等待特定响应
  * @param  expected_response: 期望的响应字节
  * @param  timeout_ms: 超时时间（毫秒）
  * @retval COM_StatusTypeDef
  * @note   重构新增：通用的响应等待函数
  */
static COM_StatusTypeDef WaitForResponse(uint8_t expected_response, uint32_t timeout_ms)
{
    uint8_t response;
    uint32_t start_time = Get_TickCount();
    
    while (GetElapsedTime(start_time) < timeout_ms) {
        if (Uart_ReadWithTimeOut(&response, 1, 10) == HAL_OK) {
            if (response == expected_response) {
                return COM_OK;
            } else if (response == CA) {
                /* 处理取消命令 */
                YMODEM_DEBUG("Transfer canceled by user");
                return COM_ABORT;
            }
        }
        Delay_ms(1);
    }
    
    return COM_TIMEOUT;
}

/**
  * @brief  接收一个数据包（重构改进版本）
  * @param  p_data: 数据缓冲区
  * @param  p_length: 接收到的数据长度
  * @param  timeout_ms: 超时时间（毫秒）
  * @retval HAL_StatusTypeDef
  * @note   重构改进：使用精确超时和更好的错误处理
  */
static COM_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout_ms)
{
    uint32_t start_time, packet_size = 0;
    uint8_t header_byte;
    COM_StatusTypeDef status = COM_OK;
    
    *p_length = 0;
    start_time = Get_TickCount();
    
    /* 等待包头 */
    while (GetElapsedTime(start_time) < timeout_ms) {
        if (Uart_ReadWithTimeOut(&header_byte, 1, 10) == HAL_OK) {
            switch (header_byte) {
                case SOH:
                    packet_size = PACKET_SIZE;
                    break;
                case STX:
                    packet_size = PACKET_1K_SIZE;
                    break;
                case EOT:
                    /* 传输结束 */
                    *p_data = header_byte;
                    *p_length = 0;
                    return COM_OK;
                case CA:
                    /* 取消传输 */
                    if (Uart_ReadWithTimeOut(&header_byte, 1, 100) == HAL_OK && header_byte == CA) {
                        *p_length = 2;
                        return COM_ABORT;
                    }
                    status = COM_PROTOCOL_ERROR;
                    break;
                case ABORT1:
                case ABORT2:
                    status = COM_ABORT;
                    break;
                default:
                    status = COM_PROTOCOL_ERROR;
                    break;
            }
            break;
        }
        Delay_ms(1);
    }
    
    if (status != COM_OK) {
        return status;
    }
    
    if (packet_size == 0) {
        return COM_PROTOCOL_ERROR; /* 未知的包头 */
    }
    
    /* 保存包头 */
    p_data[0] = header_byte;
    
    /* 接收包数据 */
    if (packet_size >= PACKET_SIZE) {
        if (Uart_ReadWithTimeOut(&p_data[PACKET_NUMBER_INDEX], 
                               packet_size + PACKET_OVERHEAD_SIZE, 
                               timeout_ms) != HAL_OK) {
            return COM_TIMEOUT;
        }
        
        /* 验证数据包 */
        status = ValidatePacket(p_data, packet_size);
        if (status == COM_OK) {
            *p_length = packet_size;
        } else {
            *p_length = 0;
        }
    }
    
    return status;
}

/**
  * @brief  准备初始包（保持原有实现）
  * @param  p_data: 数据缓冲区
  * @param  p_file_name: 文件名
  * @param  length: 文件长度
  * @retval None
  */
static void PrepareIntialPacket(uint8_t *p_data, const uint8_t *p_file_name, uint32_t length)
{
    uint32_t i, j = 0;
    uint8_t astring[10];

    p_data[PACKET_START_INDEX] = SOH;
    p_data[PACKET_NUMBER_INDEX] = 0x00;
    p_data[PACKET_CNUMBER_INDEX] = 0xff;

    for (i = 0; (p_file_name[i] != '\0') && (i < FILE_NAME_LENGTH); i++) {
        p_data[i + PACKET_DATA_INDEX] = p_file_name[i];
    }

    p_data[i + PACKET_DATA_INDEX] = 0x00;

    Int2Str(astring, length);
    i = i + PACKET_DATA_INDEX + 1;
    while (astring[j] != '\0') {
        p_data[i++] = astring[j++];
    }

    for (j = i; j < PACKET_SIZE + PACKET_DATA_INDEX; j++) {
        p_data[j] = 0;
    }
}

/**
  * @brief  准备数据包（保持原有实现）
  * @param  p_source: 源数据
  * @param  p_packet: 包缓冲区
  * @param  pkt_nr: 包序号
  * @param  size_blk: 数据块大小
  * @retval None
  */
static void PreparePacket(uint8_t *p_source, uint8_t *p_packet, uint8_t pkt_nr, uint32_t size_blk)
{
    uint8_t *p_record;
    uint32_t i, size, packet_size;

    packet_size = size_blk >= PACKET_1K_SIZE ? PACKET_1K_SIZE : PACKET_SIZE;
    size = size_blk < packet_size ? size_blk : packet_size;
    
    if (packet_size == PACKET_1K_SIZE) {
        p_packet[PACKET_START_INDEX] = STX;
    } else {
        p_packet[PACKET_START_INDEX] = SOH;
    }
    
    p_packet[PACKET_NUMBER_INDEX] = pkt_nr;
    p_packet[PACKET_CNUMBER_INDEX] = (~pkt_nr);
    p_record = p_source;

    for (i = PACKET_DATA_INDEX; i < size + PACKET_DATA_INDEX; i++) {
        p_packet[i] = *p_record++;
    }
    
    if (size <= packet_size) {
        for (i = size + PACKET_DATA_INDEX; i < packet_size + PACKET_DATA_INDEX; i++) {
            p_packet[i] = 0x1A;
        }
    }
}

/* CRC计算函数（保持原有实现） */
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte)
{
    uint32_t crc = crc_in;
    uint32_t in = byte | 0x100;

    do {
        crc <<= 1;
        in <<= 1;
        if (in & 0x100)
            ++crc;
        if (crc & 0x10000)
            crc ^= 0x1021;
    } while (!(in & 0x10000));

    return crc & 0xffffu;
}

uint16_t Cal_CRC16(const uint8_t *p_data, uint32_t size)
{
    uint32_t crc = 0;
    const uint8_t *dataEnd = p_data + size;

    while (p_data < dataEnd)
        crc = UpdateCRC16(crc, *p_data++);

    crc = UpdateCRC16(crc, 0);
    crc = UpdateCRC16(crc, 0);

    return crc & 0xffffu;
}

uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size)
{
    uint32_t sum = 0;
    const uint8_t *p_data_end = p_data + size;

    while (p_data < p_data_end) {
        sum += *p_data++;
    }

    return (sum & 0xffu);
}

/* Public functions ---------------------------------------------------------*/

/**
  * @brief  接收文件（重构改进版本）
  * @param  p_size: 文件大小指针
  * @retval COM_StatusTypeDef
  * @note   重构改进：使用状态机，更好的错误处理，进度跟踪
  */
COM_StatusTypeDef Ymodem_Receive(uint32_t *p_size)
{
    uint32_t packet_length, filesize;
    uint32_t errors = 0;
    uint8_t tmp;
    COM_StatusTypeDef result = COM_OK;
    
    /* 初始化会话 */
    ResetSession();
    ymodem_session.state = YMODEM_STATE_WAIT_HEADER;
    
    YMODEM_DEBUG("YModem receive session started");
    
    while (ymodem_session.state != YMODEM_STATE_COMPLETE && 
           ymodem_session.state != YMODEM_STATE_ERROR) {
        
        switch (ymodem_session.state) {
            case YMODEM_STATE_WAIT_HEADER:
                /* 发送CRC请求 */
                Serial_PutByte(CRC16);
                
                /* 接收数据包 */
                result = ReceivePacket(aPacketData, &packet_length, DOWNLOAD_TIMEOUT_MS);
                
                if (result == COM_OK) {
                    if (packet_length == 0) {
                        /* 空包，会话结束 */
                        Serial_PutByte(ACK);
                        ymodem_session.state = YMODEM_STATE_COMPLETE;
                        YMODEM_DEBUG("Session completed with empty packet");
                    } else {
                        /* 解析文件头 */
                        filesize = ParseFileInfo(aPacketData, packet_length);
                        
                        if (filesize > APPLICATION_FLASH_SIZE) {
                            YMODEM_DEBUG("File too large: %lu > %lu", filesize, APPLICATION_FLASH_SIZE);
                            tmp = CA;
                            Uart_WriteWithTimeOut(&tmp, 1, NAK_TIMEOUT_MS);
                            Uart_WriteWithTimeOut(&tmp, 1, NAK_TIMEOUT_MS);
                            result = COM_LIMIT;
                            ymodem_session.state = YMODEM_STATE_ERROR;
                        } else {
                            ymodem_session.file_size = filesize;
                            *p_size = filesize;
                            
                            /* 擦除Flash */
                            if (FLASH_If_Erase(APPLICATION_ADDRESS, APPLICATION_ADDRESS_END) != FLASHIF_OK) {
                                YMODEM_DEBUG("Flash erase failed");
                                result = COM_FLASH_ERROR;
                                ymodem_session.state = YMODEM_STATE_ERROR;
                            } else {
                                Serial_PutByte(ACK);
                                Serial_PutByte(CRC16);
                                ymodem_session.state = YMODEM_STATE_RECEIVING;
                                ymodem_session.packet_number = 1;
                                YMODEM_DEBUG("Header received, starting data transfer");
                            }
                        }
                    }
                } else {
                    errors++;
                    YMODEM_DEBUG("Header receive error: %d, error count: %lu", result, errors);
                }
                break;
                
            case YMODEM_STATE_RECEIVING:
                result = ReceivePacket(aPacketData, &packet_length, DOWNLOAD_TIMEOUT_MS);
                
                if (result == COM_OK) {
                    errors = 0; /* 重置错误计数 */
                    
                    if (packet_length == 0) {
                        /* EOT包 */
                        Serial_PutByte(ACK);
                        
                        /* 等待第二个EOT */
                        if (WaitForResponse(EOT, NAK_TIMEOUT_MS) == COM_OK) {
                            Serial_PutByte(ACK);
                            ymodem_session.state = YMODEM_STATE_COMPLETE;
                            YMODEM_DEBUG("File transfer completed successfully");
                        } else {
                            result = COM_PROTOCOL_ERROR;
                            ymodem_session.state = YMODEM_STATE_ERROR;
                        }
                    } else {
                        /* 数据包 */
                        if (aPacketData[PACKET_NUMBER_INDEX] != ymodem_session.packet_number) {
                            Serial_PutByte(NAK);
                            YMODEM_DEBUG("Packet sequence mismatch: expected %d, got %d", 
                                        ymodem_session.packet_number, 
                                        aPacketData[PACKET_NUMBER_INDEX]);
                        } else {
                            result = ProcessDataPacket(packet_length);
                            if (result == COM_OK) {
                                Serial_PutByte(ACK);
                            } else {
                                Serial_PutByte(CA);
                                Serial_PutByte(CA);
                                ymodem_session.state = YMODEM_STATE_ERROR;
                            }
                        }
                    }
                } else if (result == COM_ABORT) {
                    YMODEM_DEBUG("Transfer aborted");
                    ymodem_session.state = YMODEM_STATE_ERROR;
                } else {
                    errors++;
                    YMODEM_DEBUG("Data packet error: %d, error count: %lu", result, errors);
                    
                    if (errors > MAX_ERRORS) {
                        Serial_PutByte(CA);
                        Serial_PutByte(CA);
                        result = COM_ERROR;
                        ymodem_session.state = YMODEM_STATE_ERROR;
                    } else {
                        Serial_PutByte(CRC16);
                    }
                }
                break;
                
            default:
                break;
        }
        
        /* 检查总体超时 */
        if (GetElapsedTime(ymodem_session.session_start_time) > (DOWNLOAD_TIMEOUT_MS * 10)) {
            YMODEM_DEBUG("Overall session timeout");
            result = COM_TIMEOUT;
            ymodem_session.state = YMODEM_STATE_ERROR;
        }
    }
    
    if (ymodem_session.state == YMODEM_STATE_ERROR) {
        YMODEM_DEBUG("YModem receive failed with error: %d", result);
    } else {
        YMODEM_DEBUG("YModem receive completed successfully");
    }
    
    return result;
}

/**
  * @brief  发送文件（保持原有实现，仅做轻微调整）
  * @param  p_buf: 数据缓冲区
  * @param  p_file_name: 文件名
  * @param  file_size: 文件大小
  * @retval COM_StatusTypeDef
  */
COM_StatusTypeDef Ymodem_Transmit(uint8_t *p_buf, const uint8_t *p_file_name, uint32_t file_size)
{
    uint32_t errors = 0, ack_recpt = 0, size = 0, pkt_size;
    uint8_t *p_buf_int;
    COM_StatusTypeDef result = COM_OK;
    uint32_t blk_number = 1;
    uint8_t a_rx_ctrl[2];
    uint8_t i;
#ifdef CRC16_F
    uint32_t temp_crc;
#else
    uint8_t temp_chksum;
#endif

    YMODEM_DEBUG("YModem transmit started: %s, size: %lu", p_file_name, file_size);
    
    PrepareIntialPacket(aPacketData, p_file_name, file_size);

    while ((!ack_recpt) && (result == COM_OK)) {
        Uart_WriteWithTimeOut(&aPacketData[PACKET_START_INDEX], PACKET_SIZE + PACKET_HEADER_SIZE, NAK_TIMEOUT_MS);

#ifdef CRC16_F
        temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
        Serial_PutByte(temp_crc >> 8);
        Serial_PutByte(temp_crc & 0xFF);
#else
        temp_chksum = CalcChecksum(&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
        Serial_PutByte(temp_chksum);
#endif

        if (Uart_ReadWithTimeOut(&a_rx_ctrl[0], 1, NAK_TIMEOUT_MS) == HAL_OK) {
            if (a_rx_ctrl[0] == ACK) {
                ack_recpt = 1;
            } else if (a_rx_ctrl[0] == CA) {
                if ((Uart_ReadWithTimeOut(&a_rx_ctrl[0], 1, NAK_TIMEOUT_MS) == HAL_OK) && (a_rx_ctrl[0] == CA)) {
                    Delay_ms(2);
                    result = COM_ABORT;
                }
            }
        } else {
            errors++;
        }
        
        if (errors >= MAX_ERRORS) {
            result = COM_ERROR;
        }
    }

    p_buf_int = p_buf;
    size = file_size;

    while ((size) && (result == COM_OK)) {
        PreparePacket(p_buf_int, aPacketData, blk_number, size);
        ack_recpt = 0;
        a_rx_ctrl[0] = 0;
        errors = 0;

        while ((!ack_recpt) && (result == COM_OK)) {
            if (size >= PACKET_1K_SIZE) {
                pkt_size = PACKET_1K_SIZE;
            } else {
                pkt_size = PACKET_SIZE;
            }

            Uart_WriteWithTimeOut(&aPacketData[PACKET_START_INDEX], pkt_size + PACKET_HEADER_SIZE, NAK_TIMEOUT_MS);

#ifdef CRC16_F
            temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], pkt_size);
            Serial_PutByte(temp_crc >> 8);
            Serial_PutByte(temp_crc & 0xFF);
#else
            temp_chksum = CalcChecksum(&aPacketData[PACKET_DATA_INDEX], pkt_size);
            Serial_PutByte(temp_chksum);
#endif

            if ((Uart_ReadWithTimeOut(&a_rx_ctrl[0], 1, NAK_TIMEOUT_MS) == HAL_OK) && (a_rx_ctrl[0] == ACK)) {
                ack_recpt = 1;
                if (size > pkt_size) {
                    p_buf_int += pkt_size;
                    size -= pkt_size;
                    if (blk_number == (APPLICATION_FLASH_SIZE / PACKET_1K_SIZE)) {
                        result = COM_LIMIT;
                    } else {
                        blk_number++;
                    }
                } else {
                    p_buf_int += pkt_size;
                    size = 0;
                }
            } else {
                errors++;
            }

            if (errors >= MAX_ERRORS) {
                result = COM_ERROR;
            }
        }
    }

    ack_recpt = 0;
    a_rx_ctrl[0] = 0x00;
    errors = 0;
    
    while ((!ack_recpt) && (result == COM_OK)) {
        Serial_PutByte(EOT);

        if (Uart_ReadWithTimeOut(&a_rx_ctrl[0], 1, NAK_TIMEOUT_MS) == HAL_OK) {
            if (a_rx_ctrl[0] == ACK) {
                ack_recpt = 1;
            } else if (a_rx_ctrl[0] == CA) {
                if ((Uart_ReadWithTimeOut(&a_rx_ctrl[0], 1, NAK_TIMEOUT_MS) == HAL_OK) && (a_rx_ctrl[0] == CA)) {
                    Delay_ms(2);
                    result = COM_ABORT;
                }
            }
        } else {
            errors++;
        }

        if (errors >= MAX_ERRORS) {
            result = COM_ERROR;
        }
    }

    if (result == COM_OK) {
        aPacketData[PACKET_START_INDEX] = SOH;
        aPacketData[PACKET_NUMBER_INDEX] = 0;
        aPacketData[PACKET_CNUMBER_INDEX] = 0xFF;
        
        for (i = PACKET_DATA_INDEX; i < (PACKET_SIZE + PACKET_DATA_INDEX); i++) {
            aPacketData[i] = 0x00;
        }

        Uart_WriteWithTimeOut(&aPacketData[PACKET_START_INDEX], PACKET_SIZE + PACKET_HEADER_SIZE, NAK_TIMEOUT_MS);

#ifdef CRC16_F
        temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
        Serial_PutByte(temp_crc >> 8);
        Serial_PutByte(temp_crc & 0xFF);
#else
        temp_chksum = CalcChecksum(&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
        Serial_PutByte(temp_chksum);
#endif

        if (Uart_ReadWithTimeOut(&a_rx_ctrl[0], 1, NAK_TIMEOUT_MS) == HAL_OK) {
            if (a_rx_ctrl[0] == CA) {
                Delay_ms(2);
                result = COM_ABORT;
            }
        }
    }
    
    YMODEM_DEBUG("YModem transmit completed with result: %d", result);
    
    return result;
}

/* 硬件抽象层函数实现示例 */
/**
  * @brief  带超时的串口读取
  * @param  data: 数据缓冲区
  * @param  len: 数据长度
  * @param  timeOut_ms: 超时时间（毫秒）
  * @retval HAL_StatusTypeDef
  * @note   需要用户根据具体硬件实现
  */
static HAL_StatusTypeDef Uart_ReadWithTimeOut(uint8_t *data, uint16_t len, uint32_t timeOut_ms)
{
    uint32_t start_time = Get_TickCount();
    uint16_t bytes_read = 0;
    
    while (GetElapsedTime(start_time) < timeOut_ms) {
        /* 这里需要实现具体的串口读取逻辑 */
        /* 示例：if (UART_Receive(data + bytes_read, len - bytes_read) > 0) ... */
        
        if (bytes_read >= len) {
            return HAL_OK;
        }
        Delay_ms(1);
    }
    
    return HAL_TIMEOUT;
}

/**
  * @brief  带超时的串口写入
  * @param  data: 数据缓冲区
  * @param  len: 数据长度
  * @param  timeOut_ms: 超时时间（毫秒）
  * @retval HAL_StatusTypeDef
  * @note   需要用户根据具体硬件实现
  */
static HAL_StatusTypeDef Uart_WriteWithTimeOut(uint8_t *data, uint16_t len, uint32_t timeOut_ms)
{
    /* 这里需要实现具体的串口写入逻辑 */
    /* 示例：return UART_Transmit(data, len) == len ? HAL_OK : HAL_ERROR; */
    return HAL_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

