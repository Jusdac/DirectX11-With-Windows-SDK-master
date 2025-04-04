cmake_minimum_required(VERSION 3.14)

#   ӱ  붨  
add_compile_definitions(UNICODE _UNICODE)

#    ֧   Windows 7 ϵͳ         _WIN32_WINNT   
if(WIN7_SYSTEM_SUPPORT MATCHES ON)
    add_compile_definitions(_WIN32_WINNT=0x601)
endif()

# Ϊ MSVC     UTF-8     ѡ  
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

#  ռ   ĿԴ ļ 
aux_source_directory(. DIR_SRCS)
file(GLOB HLSL_FILES HLSL/*.hlsl HLSL/*.hlsli)
file(GLOB HEADER_FILES ./*.h)

#      HLSL  ļ         ɫ      
foreach(HLSL_FILE ${HLSL_FILES})
    get_filename_component(HLSL_FDIR ${HLSL_FILE} DIRECTORY)
    get_filename_component(HLSL_FBASENAME_WE ${HLSL_FILE} NAME_WE)
    string(CONCAT HLSL_FNAME_WE ${HLSL_FDIR} / ${HLSL_FBASENAME_WE})

    string(LENGTH ${HLSL_FBASENAME_WE} LEN_FNAME_WE)
    math(EXPR LEN_FNAME_WE "${LEN_FNAME_WE}-2")
    string(SUBSTRING ${HLSL_FBASENAME_WE} ${LEN_FNAME_WE} 2 ENTRY_POINT)
    string(TOLOWER ${ENTRY_POINT} SHADER_TYPE)

    if("${SHADER_TYPE}" STREQUAL "vs")
        set(SHADER_TYPE "Vertex")
    elseif("${SHADER_TYPE}" STREQUAL "hs")
        set(SHADER_TYPE "Hull")
    elseif("${SHADER_TYPE}" STREQUAL "ds")
        set(SHADER_TYPE "Domain")
    elseif("${SHADER_TYPE}" STREQUAL "gs")
        set(SHADER_TYPE "Geometry")
    elseif("${SHADER_TYPE}" STREQUAL "ps")
        set(SHADER_TYPE "Pixel")
    elseif("${SHADER_TYPE}" STREQUAL "cs")
        set(SHADER_TYPE "Compute")
    endif()

    set_source_files_properties(${HLSL_FILE} PROPERTIES
        VS_SHADER_OBJECT_FILE_NAME HLSL/${HLSL_FBASENAME_WE}.cso
        VS_SHADER_TYPE ${SHADER_TYPE}
        VS_SHADER_MODEL 5.0
        VS_SHADER_ENTRYPOINT ${ENTRY_POINT}
        VS_SHADER_DISABLE_OPTIMIZATIONS $<$<CONFIG:Debug>:true>
        VS_SHADER_ENABLE_DEBUG $<$<CONFIG:Debug>:true>)
endforeach()

#     ImGui   
aux_source_directory(. IMGUI_DIR_SRCS)
add_library(ImGui STATIC ${IMGUI_DIR_SRCS})
target_include_directories(ImGui PUBLIC .)


add_executable(06_Use_ImGui WIN32 ${DIR_SRCS} ${HEADER_FILES} ${HLSL_FILES})
target_link_libraries(06_Use_ImGui d3d11.lib dxgi.lib dxguid.lib D3DCompiler.lib winmm.lib ImGui)

#      imgui.ini  ļ      Ŀ¼
file(COPY imgui.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

#     Դ ļ     
source_group("HLSL Files" FILES ${HLSL_FILES})

#     Ŀ      
set_target_properties(06_Use_ImGui PROPERTIES OUTPUT_NAME "06 Use ImGui")
set_target_properties(06_Use_ImGui PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(06_Use_ImGui PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(06_Use_ImGui PROPERTIES FOLDER "Project 01-09")