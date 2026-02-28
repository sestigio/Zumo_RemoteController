idf_component_register(SRCS "uart_component.c"
                       INCLUDE_DIRS "include" # O "." si el .h está junto al .c
                       REQUIRES driver) # Dependencias que use este código