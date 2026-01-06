#    Usage example
#    # Extract giflib
#    extract_archive_flat(
#        INPUT "${CMAKE_SOURCE_DIR}/third_party/giflib-5.2.2.tar.gz"
#        DESTINATION "${CMAKE_SOURCE_DIR}/third_party/giflib"
#    )
#    
#    # Extract opencv
#    extract_archive_flat(
#        INPUT "${CMAKE_SOURCE_DIR}/third_party/opencv-4.9.0.zip"
#        DESTINATION "${CMAKE_SOURCE_DIR}/third_party/opencv"
#    )
function(extract_archive_flat archive_path dest_dir)
    # Generate a unique temp path (based on the input parameter hash)
    string(MD5 archive_hash "${archive_path}-${dest_dir}")
    set(extract_temp "${CMAKE_CURRENT_BINARY_DIR}/_extract_temp_${archive_hash}")

    # Extract to the temp directory
    file(ARCHIVE_EXTRACT
        INPUT "${archive_path}"
        DESTINATION "${extract_temp}"
    )

    # Check if there is only one subdirectory
    file(GLOB top_entries RELATIVE "${extract_temp}" "${extract_temp}/*")
    list(LENGTH top_entries entry_count)
    list(GET top_entries 0 maybe_top_dir)

    if(entry_count EQUAL 1 AND IS_DIRECTORY "${extract_temp}/${maybe_top_dir}")
        # If the unpacked content is in a top-level directory, flatten-copy its contents
        file(GLOB_RECURSE files "${extract_temp}/${maybe_top_dir}/*")
        foreach(file IN LISTS files)
            file(RELATIVE_PATH rel "${extract_temp}/${maybe_top_dir}" "${file}")
            get_filename_component(out_dir "${dest_dir}/${rel}" DIRECTORY)
            file(MAKE_DIRECTORY "${out_dir}")
            file(COPY "${file}" DESTINATION "${out_dir}")
        endforeach()
    else()
        # Otherwise copy all contents directly
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
