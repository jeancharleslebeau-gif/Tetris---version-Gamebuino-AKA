set(COMPONENT_SRCS
    app_main.cpp
    task_game.cpp
    audio_pmf.cpp
    game.cpp
    game_state.cpp
    audio.cpp
    game_logic.cpp
    render.cpp
)

set(COMPONENT_ADD_INCLUDEDIRS ".")

idf_component_register(
    SRCS ${COMPONENT_SRCS}
    INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}
    REQUIRES core gamebuino
)

# Forcer le composant à être compilé en C++
set_property(TARGET ${COMPONENT_LIB} PROPERTY LINKER_LANGUAGE CXX)
