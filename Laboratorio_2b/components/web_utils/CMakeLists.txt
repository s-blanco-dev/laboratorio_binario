idf_component_register(
    SRCS "web_utils.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_http_server json
    EMBED_FILES
        "web/index.html"
        "web/style.css"
        "web/app.js"
)
