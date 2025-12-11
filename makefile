# Directorios de origen y destino
SRC_DIR := src
BIN_DIR := bin

SFML := -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# Archivos fuente del juego Snake
GAME_SRC := $(SRC_DIR)/04_Main.cpp $(SRC_DIR)/01_Snake.cpp $(SRC_DIR)/02_Barrier.cpp $(SRC_DIR)/03_GameLogic.cpp $(SRC_DIR)/06_SnakeRenderer.cpp
GAME_EXE := $(BIN_DIR)/Snake.exe

# Regla por defecto para compilar el juego
all: $(GAME_EXE)

# Compilar el ejecutable del juego
$(GAME_EXE): $(GAME_SRC)
	g++ $(GAME_SRC) -o $@ $(SFML) -Iinclude

# Ejecutar el juego
run: $(GAME_EXE)
	./$<

# Limpiar los archivos generados
clean:
	rm -f $(GAME_EXE)

.PHONY: all clean run