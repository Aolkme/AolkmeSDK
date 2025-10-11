idf_component_register(
                    SRCS

                    INCLUDE_DIRS 
                        "include"
                    SRC_DIRS
                        "src"
                    PRIV_INCLUDE_DIRS
                        "internal")


# 禁用未使用变量的警告
target_compile_options(${COMPONENT_LIB} PRIVATE 
    "-Wno-unused-variable"
    "-Wno-unused-const-variable"
)


