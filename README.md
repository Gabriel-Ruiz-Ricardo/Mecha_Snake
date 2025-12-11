# Snake++
Videojuego clásico de Snake con barreras rectangular cerrada que encierra la serpiente.
Proyecto final de Programación Avanzada. 
Centro de Enseñanza Técnica Industrial.
Grupo: 3E.     Registros: 24310398 - 24310399.

## Características
- **Serpiente controlable**: Usa flechas o WASD para mover
- **Barreras rectangulares**: Forman un rectángulo cerrado que encierra el área de juego
- **Comida**: Come items rojos para crecer y aumentar puntuación
- **Colisión con barreras**: Choca contra los muros blancos → Game Over
- **Colisión consigo misma**: Game Over si la serpiente se choca
- **Reinicio**: Presiona R después de Game Over para reiniciar

## Estructura del Proyecto
```
Mecha_Snake/
├── include/
│   ├── Snake.hpp        # Declaración clase Snake
│   ├── Barrier.hpp      # Declaración clase Barrier (barreras rectangulares)
│   ├── GameLogic.hpp    # Declaración clase GameLogic
│   └── Common.hpp       # Definiciones comunes (struct Cell)
├── src/
│   ├── 04_Main.cpp      # Punto de entrada y loop principal
│   ├── 01_Snake.cpp     # Implementación Snake
│   ├── 02_Barrier.cpp   # Implementación Barrier
│   └── 03_GameLogic.cpp # Implementación GameLogic
├── bin/
│   └── Snake.exe        # Ejecutable compilado
├── makefile             # Archivo de compilación
└── README.md            # Este archivo
```

## Compilación
Asegúrate de tener SFML instalado. Luego ejecuta:
```bash
make
```

O compila manualmente:
```bash
g++ src/04_Main.cpp src/01_Snake.cpp src/02_Barrier.cpp src/03_GameLogic.cpp -o bin/Snake.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -Iinclude
```

## Ejecución
```bash
.\bin\Snake.exe
```

## Controles
- **Flechas o WASD**: Mover la serpiente
- **R**: Reiniciar el juego después de Game Over
- **ESC**: Salir del juego

## Notas Técnicas
- Las barreras forman un rectángulo cerrado que contiene toda el área de juego
- La serpiente comienza en el centro del rectángulo
- La comida solo aparece dentro del área delimitada por las barreras
- No hay envoltura de bordes; la serpiente debe evitar las paredes

