# 🐍 Mecha-Snake 🚪

## 🎯 OBJETIVO
Llevar al usuario a través del clásico juego de Snake, mientras se aventura hacia portales que lo transportarán a **nuevos mapas**.

---

## 🎮 CONTROLES
El movimiento de la serpiente puede controlarse con dos conjuntos de teclas:
* **W/A/S/D**
* **↑/↓/→/←**

---

## 🛠️ MÉCANICAS

### Mécanicas de Juego Base
* **Movimiento:** El jugador controla la dirección de la serpiente (**Arriba, Abajo, Izquierda, Derecha**).
* **Crecimiento:** Cuando la serpiente consume un objeto (fruta), su longitud aumenta en **una unidad**.
* **Objetivo Principal:** Consumir tantas frutas como sea posible para hacer crecer la serpiente y conseguir **puntos**.
* **Condiciones de Derrota:**
    * **Choque con Muro:** La serpiente golpea los límites del área de juego.
    * **Choque con Cola:** La serpiente golpea su propio cuerpo.

### Mécanicas de Frutas y Puntuación
* **Frutas de Puntuación Variable:** Existirán tres tipos de frutas, cada una otorgando una cantidad de puntos distinta.
    * *Ejemplo:* **Goma goma** = 1 punto, **Mero mero** = 5 puntos, **Upe upe** = 10 puntos.
* **Puntuación Acumulativa:** Se mantiene un contador de puntos total. El objetivo es alcanzar **30 puntos para abrir el portal**.
* **Contador de Recolección:** Cada fruta consumida aumenta un contador que permite seguir recolectando.

> [!NOTE]
> Es importante diferenciar entre el **Puntaje** (la meta de 30) y el **Crecimiento/Progreso** (la longitud de la serpiente). Una fruta de 5 puntos haría crecer la serpiente lo mismo que una de 1 punto, pero te acercaría más rápido al portal.

---

## ✨ CARACTERÍSTICAS
* **Juego de Habilidad y Reflejos:** Un núcleo de juego rápido y reactivo basado en el control direccional en una cuadrícula.
* **Progresión por Puntuación:** El avance a través de los niveles no depende de la longitud de la serpiente, sino de un **contador de puntos acumulados**.
* **Riesgo vs. Recompensa en Recolección:** Introduce la elección estratégica de buscar frutas de alto valor (ej. 5 puntos) para alcanzar el portal más rápido, versus frutas de bajo valor (ej. 1 punto) que son potencialmente más fáciles de alcanzar.
* **Posiciones Aleatorias:** Tanto las frutas como el portal aparecen en posiciones aleatorias dentro del mapa disponible, manteniendo la imprevisibilidad.

---

## 💻 TECNOLOGÍAS
* **Lenguaje:** C++
* **Librerías adicionales:** SMF

---

## 👥 EQUIPO
* **Líder:** Gabriel Alejandro Ruiz Ricardo (@Gabriel-Ruiz-Ricardo)
* **Integrante:** Zayra Elizabeth Rivera Mendoza (@Elizabeth398)

---

## 📄 CREDITOS

### Assets
* **Música: Snow City Theme:** https://opengameart.org/content/snow-city-theme
* **Fuente Minecraft:** https://www.dafont.com/es/minecraft.font
* **Fuente Homoarakhan:** https://www.dafont.com/font-comment.php?file=homoarakhan

### Referencia
* Snake

### Agradecimientos
* Prof. Jose Ramon Navarro - Ceti Colomos