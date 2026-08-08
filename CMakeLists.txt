cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(modbus-replacer-stm32)

target_sources(app PRIVATE src/main.c)

set(gen_dir ${ZEPHYR_BINARY_DIR}/include/generated/)

zephyr_linker_sources(SECTIONS sections-rom.ld)
zephyr_linker_section(
	NAME http_resource_desc_modbus_replacer_http_service
 	KVMA RAM_REGION GROUP RODATA_REGION
)
foreach(web_resource
  index.html
)
  generate_inc_file_for_target(app
    src/web/${web_resource}
    ${gen_dir}/${web_resource}.gz.inc
    --gzip
  )
endforeach()
