target_sources(synfigstudio
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/renderer_background.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderer_bbox.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderer_canvas.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderer_dragbox.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderer_ducks.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderer_frameerror.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderer_grid.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderer_guides.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderer_timecode.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderer_bonesetup.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderer_bonedeformarea.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/workarearenderer.cpp"
)
