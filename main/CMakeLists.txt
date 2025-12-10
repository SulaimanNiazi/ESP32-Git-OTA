idf_component_register(
    SRCS "ota.c" "wifi.c" "nvs.c" "delay.c" "gpio.c" "uart.c" "main.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES "../lib/certificate.pem"
)
