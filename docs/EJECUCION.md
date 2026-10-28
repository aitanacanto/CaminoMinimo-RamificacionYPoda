# Compilación y ejecución

Necesitas un compilador C++17. El Makefile utiliza órdenes de un shell Unix; en Windows se puede usar WSL/MSYS2 o compilar directamente con un `g++` instalado:

```powershell
New-Item -ItemType Directory -Force build
g++ -Wall -O3 -std=c++17 -o build/mcp_bb.exe src/mcp_bb.cc
.\build\mcp_bb.exe -f tests/data/002.map --p2D -p
python tests/check_cases.py --binary build/mcp_bb.exe
```

En Linux/macOS con las herramientas correspondientes: `make` y `make check`.

## Entrada y salida

El fichero contiene filas y columnas, seguidas de los pesos enteros positivos de las celdas. Se permite moverse a cualquiera de las ocho casillas vecinas. El coste incluye origen y destino.

La salida principal muestra coste, estadísticas y tiempo. `-p` añade el camino codificado y `--p2D` lo dibuja en la cuadrícula. `-f` proporciona el fichero.

El comprobador compara la primera línea con la salida docente de referencia. Tiene un límite de tiempo por caso y no valida todavía la reconstrucción ni cada contador.
