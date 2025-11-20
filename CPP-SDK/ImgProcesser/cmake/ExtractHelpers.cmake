#    用法示例
#    # 解压 giflib
#    extract_archive_flat(
#        INPUT "${CMAKE_SOURCE_DIR}/third_party/giflib-5.2.2.tar.gz"
#        DESTINATION "${CMAKE_SOURCE_DIR}/third_party/giflib"
#    )
#    
#    # 解压 opencv
#    extract_archive_flat(
#        INPUT "${CMAKE_SOURCE_DIR}/third_party/opencv-4.9.0.zip"
#        DESTINATION "${CMAKE_SOURCE_DIR}/third_party/opencv"
#    )
function(extract_archive_flat archive_path dest_dir)
    # 生成唯一的临时路径（基于传入参数的 hash）
    string(MD5 archive_hash "${archive_path}-${dest_dir}")
    set(extract_temp "${CMAKE_CURRENT_BINARY_DIR}/_extract_temp_${archive_hash}")

    # 解压到临时目录
    file(ARCHIVE_EXTRACT
        INPUT "${archive_path}"
        DESTINATION "${extract_temp}"
    )

    # 判断是否只有一个子目录
    file(GLOB top_entries RELATIVE "${extract_temp}" "${extract_temp}/*")
    list(LENGTH top_entries entry_count)
    list(GET top_entries 0 maybe_top_dir)

    if(entry_count EQUAL 1 AND IS_DIRECTORY "${extract_temp}/${maybe_top_dir}")
        # 解包内容在一个顶层目录内，扁平化拷贝它的内容
        file(GLOB_RECURSE files "${extract_temp}/${maybe_top_dir}/*")
        foreach(file IN LISTS files)
            file(RELATIVE_PATH rel "${extract_temp}/${maybe_top_dir}" "${file}")
            get_filename_component(out_dir "${dest_dir}/${rel}" DIRECTORY)
            file(MAKE_DIRECTORY "${out_dir}")
            file(COPY "${file}" DESTINATION "${out_dir}")
        endforeach()
    else()
        #define 否则直接复制所有内容
        file(GLOB_RECURSE files "${extract_temp}/*")
        foreach(file IN LISTS files)
            file(RELATIVE_PATH rel "${extract_temp}" "${file}")
            get_filename_component(out_dir "${dest_dir}/${rel}" DIRECTORY)
            file(MAKE_DIRECTORY "${out_dir}")
            file(COPY "${file}" DESTINATION "${out_dir}")
        endforeach()
    endif()

    message(STATUS "Extracted ${archive_path} → ${dest_dir}")
endfunction()
