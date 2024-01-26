set(ML_MODULE  ML)

aux_source_directory(.  ML_SOURCE)
add_library(${ML_MODULE} STATIC ${ML_SOURCE})

target_link_libraries(${ML_MODULE} tensorflow-lite ${OpenCV_LIBS} m)
