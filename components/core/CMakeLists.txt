idf_component_register(
    SRCS
        "graphics.cpp"
        "persist.cpp"
	"input.cpp"
        "filesystem.cpp"   
    INCLUDE_DIRS "."
    PRIV_REQUIRES gamebuino
)
