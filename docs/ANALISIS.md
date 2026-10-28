# Análisis y alcance

## Diseño

El mapa se almacena como matriz de pesos. La solución utiliza una cola de prioridad, matrices de costes y caminos, y contadores de nodos. Una solución inicial por programación dinámica proporciona una cota de coste, mientras que la ramificación explora las ocho direcciones y descarta estados no factibles o no prometedores.

## Experimentos de la memoria

La memoria original compara el uso de la matriz de control `noIter` con una variante sin ella y comunica diferencias de tiempo en mapas de distintos tamaños. Esos tiempos son históricos; no se han vuelto a medir ni se conserva aquí una variante independiente sin esa optimización para repetir la comparación.

## Comprobaciones pendientes

- Contrastar los costes con todos los casos de referencia, incluidos mapas grandes y no cuadrados.
- Validar que el camino reconstruido siempre tiene el coste anunciado, además del coste de la primera línea.
- Revisar el tratamiento del camino cuando la cota inicial ya es óptima: la reconstrucción depende de estructuras que se actualizan durante la búsqueda.
- Comprobar cotas y estadísticas antes de afirmar optimalidad en entradas arbitrarias. El código no realiza una validación completa de ficheros mal formados.

La [memoria de la práctica](memoria.pdf) está disponible como documentación complementaria del diseño y los experimentos.
