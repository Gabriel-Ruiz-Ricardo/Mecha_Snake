# 🐍 Mecha-Snake 🚪

## 🎯 OBJETIVO
[cite_start]Llevar al usuario a través del clásico juego de Snake, mientras se aventura hacia portales que lo transportarán a **nuevos mapas**[cite: 1].

---

## 🎮 CONTROLES
El movimiento de la serpiente puede controlarse con dos conjuntos de teclas:
* [cite_start]**W/A/S/D** [cite: 2]
* [cite_start]**↑/↓/→/←** [cite: 2]

---

## 🛠️ MÉCANICAS

### Mécanicas de Juego Base
* [cite_start]**Movimiento:** El jugador controla la dirección de la serpiente (**Arriba, Abajo, Izquierda, Derecha**)[cite: 2].
* [cite_start]**Crecimiento:** Cuando la serpiente consume un objeto (fruta), su longitud aumenta en **una unidad**[cite: 3].
* [cite_start]**Objetivo Principal:** Consumir tantas frutas como sea posible para hacer crecer la serpiente y conseguir **puntos**[cite: 4].
* **Condiciones de Derrota:**
    * [cite_start]**Choque con Muro:** La serpiente golpea los límites del área de juego[cite: 5].
    * [cite_start]**Choque con Cola:** La serpiente golpea su propio cuerpo[cite: 6].

### Mécanicas de Frutas y Puntuación
* [cite_start]**Frutas de Puntuación Variable:** Existirán tres tipos de frutas, cada una otorgando una cantidad de puntos distinta[cite: 6].
    * [cite_start]*Ejemplo:* **Goma goma** = 1 punto, **Mero mero** = 5 puntos, **Upe upe** = 10 puntos[cite: 6].
* **Puntuación Acumulativa:** Se mantiene un contador de puntos total. [cite_start]El objetivo es alcanzar **30 puntos para abrir el portal**[cite: 7].
* [cite_start]**Contador de Recolección:** Cada fruta consumida aumenta un contador que permite seguir recolectando[cite: 8].

> [!NOTE]
> [cite_start]Es importante diferenciar entre el **Puntaje** (la meta de 30) y el **Crecimiento/Progreso** (la longitud de la serpiente)[cite: 9]. [cite_start]Una fruta de 5 puntos haría crecer la serpiente lo mismo que una de 1 punto, pero te acercaría más rápido al portal[cite: 10].

---

## ✨ CARACTERÍSTICAS
* [cite_start]**Juego de Habilidad y Reflejos:** Un núcleo de juego rápido y reactivo basado en el control direccional en una cuadrícula[cite: 11].
* [cite_start]**Progresión por Puntuación:** El avance a través de los niveles no depende de la longitud de la serpiente, sino de un **contador de puntos acumulados**[cite: 12].
* [cite_start]**Riesgo vs. Recompensa en Recolección:** Introduce la elección estratégica de buscar frutas de alto valor (ej. 5 puntos) para alcanzar el portal más rápido, versus frutas de bajo valor (ej. 1 punto) que son potencialmente más fáciles de alcanzar[cite: 13].
* [cite_start]**Posiciones Aleatorias:** Tanto las frutas como el portal aparecen en posiciones aleatorias dentro del mapa disponible, manteniendo la imprevisibilidad[cite: 14].

---

## 💻 TECNOLOGÍAS
* **Lenguaje:** C++
* **Librerías adicionales:** SMF

---

## 👥 EQUIPO
* [cite_start]**Líder:** Gabriel Alejandro Ruiz Ricardo (@Gabriel-Ruiz-Ricardo) [cite: 15]
* [cite_start]**Integrante:** Zayra Elizabeth Rivera Mendoza (@Elizabeth398) [cite: 15]

---

## 📄 CREDITOS

### Assets
* [cite_start]**Música:** opengameart.org [cite: 16]
* [cite_start]**Fuente Minecraft:** https://www.dafont.com/es/minecraft.font [cite: 16]
* [cite_start]**Fuente Homoarakhan:** https://www.dafont.com/font-comment.php?file=homoarakhan [cite: 16]

### Referencia
* Snake

### Agradecimientos
* Prof. Jose Ramon Navarro - Ceti Colomos