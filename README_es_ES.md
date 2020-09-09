# sm64pc
Adaptación a OpenGL de [n64decomp/sm64](https://github.com/n64decomp/sm64). 

No dudes en contribuir o reportar bugs, pero recuerda: **no se debe subir nada con copyright**. 
Ejecuta `./extract_assets.py --clean && make clean` o `make distclean` para borrar todo el contenido proveniente de la ROM. Este port es posible gracias a [n64-fast32-engine](https://github.com/Emill/n64-fast3d-engine/) creado por [Emill](https://github.com/Emill).

## Funcionalidades

 * Renderizado nativo. Podrás jugar a Super Mario 64 sin necesidad de un emulador. 
 * Resolución y relación de aspecto variables. Puedes jugar a Super Mario 64 a básicamente cualquier resolución o tamaño de ventana.
 * Soporte nativo para mandos XInput. En Linux, se ha confirmado que el DualShock 4 funciona sin más.
 * Cámara analógica y cámara controlada con el ratón. (Se activa con `make BETTERCAMERA=1`.)
 * Opción para desactivar el límite de distancia de renderizado. (Se activa con `make NODRAWINGDISTANCE=1`.)
 * Configurar los controles desde el juego.
 * Posibilidad de saltarte la intro con la opción de línea de comandos `--skip-intro`
 * Menú de trucos (_cheats_) en _options_. (Se activa con la opción de línea de comandos `--cheats`) Ten en cuenta que si un cheat te pide pulsar el botón "L", se refiere al botón de N64, el cual tendrá que estar asignado a un botón de tu mando. Ve a los ajustes de control y asegúrate de que tienes "L" mapeado a un botón de tu mando.

## Compilar en Windows
**No intentes compilar ejecutables para Windows bajo Linux usando `WINDOWS_BUILD=1`. No va a funcionar. Sigue la guía.**
#### 1. Instalación y configuración de MSYS2.

1. Descarga [msys2-x86_64-latest.exe](http://repo.msys2.org/distrib/msys2-x86_64-latest.exe) y ejecútalo. Si tu sistema operativo es de 32 bits (¿por qué?) descarga [msys2-i686-latest.exe](http://repo.msys2.org/distrib/msys2-i686-latest.exe) en su lugar. Asegúrate de que lo instalas en `C:\dev\msys64` (o `C:\dev\msys32` para 32 bits...). Ejecuta MSYS2.

2. En la ventana de comandos de MSYS2, ejecuta el siguiente comando:
    ```
   pacman -Syuu
    ```
3. Abre "MSYS2 MinGW 64-Bit". Ejecuta este comando __repetidamente__ hasta que MSYS diga que ya no hay más actualizaciones. Es posible que tengas que volver a cerrar y abrir MSYS2.

   ```
   pacman -Syuu
   ```

5. Ejecuta este comando y cuando te pida confirmación pulsa intro:

   ```
   pacman -S --needed base-devel mingw-w64-i686-toolchain mingw-w64-x86_64-toolchain \
                       git subversion mercurial \
                       mingw-w64-i686-cmake mingw-w64-x86_64-cmake
   ```
6. Listo.
#### Instala las dependencias
```
pacman -S mingw-w64-i686-glew mingw-w64-x86_64-glew mingw-w64-i686-SDL2 mingw-w64-x86_64-SDL2 python3
```
### Crea el directorio en el que preparar todo
Desde el explorador de Windows, navega a `C:\msys64\home\(nombre de usuario)\` y crea una carpeta con el nombre que te apetezca. Aquí es donde vamos a preparar todo.
### Clona el repositorio
En MSYS2, introduce el siguiente comando:
```
git clone https://github.com/sm64pc/sm64pc/
```
(Si no funciona, prueba a escribirlo manualmente, en lugar de copiar y pegar)
#### Copia la ROM base al directorio correspondiente
El paso anterior tiene que haber creado una carpeta llamada sm64pc. Dentro de esa carpeta, y para cada version de la ROM (jp/us/eu) de la cual quieras compilar un ejecutable, coloca la ROM con el nombre `baserom.<version>.z64` para extraer sus assets. Por ejemplo, `baserom.us.z64` para la versión americana, o `baserom.eu.z64` para la versión europea.

#### En MSYS2, vamos a navegar a la carpeta `./tools/audiofile-0.3.6/` y ejecutar el `autoreconf-i`. Introduce los siguientes comandos, en orden, uno a uno:
```
cd sm64pc/tools/audiofile-0.3.6/
autoreconf -i
```
No te vayas de este directorio hasta el paso 9.

#### Ejecuta el script `configure`
```
PATH=/mingw64/bin:/mingw32/bin:$PATH LIBS=-lstdc++ ./configure --disable-docs
```
#### Ejecuta el script `make`
```
PATH=/mingw64/bin:/mingw32/bin:$PATH make
```
#### Crea un directorio `lib` en `tools/`
```
mkdir ../lib
```

#### Acabas de compilar `libaudiofile`. Ahora cópialo a `tools/lib/`
```
cp libaudiofile/.libs/libaudiofile.a ../lib/
cp libaudiofile/.libs/libaudiofile.la ../lib/
```
#### Ahora toca hacer algo desde Windows. 
En el explorador de Windows, ve a sm64pc\tools y edita el archivo Makefile desde un editor de texto (es recomendable usar un editor decente como Notepad++ o Sublime Text, en lugar del bloc de notas, para asegurarte de que no rompes el formato del texto) Busca la línea que contiene esto:

```tabledesign_CFLAGS := -Wno-uninitialized -laudiofile```

Y añade ` -lstdc++` al final, de manera que quede así (¡no olvides el espacio!)

```tabledesign_CFLAGS := -Wno-uninitialized -laudiofile -lstdc++```

Guarda el archivo.
#### Vuelve a la carpeta tools y ejecuta `make` con los siguientes comandos.
```
cd ..
PATH=/mingw64/bin:/mingw32/bin:$PATH make
```
#### Vuelve al directorio sm64pc
```
cd ..
```
#### Finalmente, ejecuta ```make``` de nuevo. 

(Ten en cuenta que mingw32 y mingw64 han sido intercambiados. Esto es para que puedas compilar la versión de 32 bits si quieres.)

Aquí pones las opciones que quieras según la versión que quieras compilar. Por ejemplo, si quieres activar la cámara analógica, añade al final BETTERCAMERA=1. Si quieres la opción de distancia de renderizado ilimitada, añade NODRAWINGDISTANCE=1.

Por ejemplo:
```
PATH=/mingw32/bin:/mingw64/bin:$PATH make BETTERCAMERA=1 NODRAWINGDISTANCE=1
```
Listo. El .exe estará en sm64pc\build\. Disfruta.
## Compilar en Linux

### Nota para usuarios de Windows
No intentes compilar un ejecutable para Windows desde Linux o WSL. No funciona. Sigue la guía para Windows.

#### Copia la(s) ROM(s) base para la extracción de assets.

Por cada versión de la cual quieras compilar un ejecutable, copia la ROM en `./baserom.<versión>.z64` para extraer los assets.

#### Instala las dependencias.

Para compilar necesitas las sigueintes dependencias.
  * python3 >= 3.6
  * libsdl2-dev
  * [audiofile](https://audiofile.68k.org/)
  * libglew-dev
  * git

Puedes instalarlas con este comando:

##### Debian / Ubuntu - (compilando para 32 bits)
```
sudo apt install build-essential git python3 libaudiofile-dev libglew-dev:i386 libsdl2-dev:i386
```
##### Debian / Ubuntu - (compilando para 64 bits)
```
sudo apt install build-essential git python3 libaudiofile-dev libglew-dev libsdl2-dev
```
##### Arch Linux
Hay un paquete AUR (cortesía de @narukeh) disponible bajo el nombre [sm64pc-git](https://aur.archlinux.org/packages/sm64pc-git/). Instálalo con tu gestor de AURs preferido.

Si quieres compilarlo por tu cuenta:
```
sudo pacman -S base-devel python audiofile sdl2 glew
```

##### Void Linux - (compilando para 64 bits)
```
sudo xbps-install -S base-devel python3 audiofile-devel SDL2-devel glew-devel
```

##### Void Linux - (compilando para 32 bits)
```
sudo xbps-install -S base-devel python3 audiofile-devel-32bit SDL2-devel-32bit glew-devel-32bit
```

##### Alpine Linux - (compilando para 32 bits y 64 bits)
```
sudo apk add build-base python3 audiofile-dev sdl2-dev glew-dev
```

#### Compila el ejecutable.

Ejecuta `make` para compilar (por defecto `VERSION=us`)

```
make VERSION=jp -j6                           # Compila la versión (J) usando 6 hilos
make VERSION=us MARCH=i686 TARGET_BITS=32     # Compila un ejecutable de la versión (U) de 32 bits
make TARGET_RPI=1                             # Compila un ejecutable para Raspberry Pi
```
## Compilar para la web
Puedes compilar el juego para navegadores que admitan WebGL usando [Emscripten](https://github.com/emscripten-core). Para hacerlo, instala [emsdk](https://github.com/emscripten-core/emsdk) y ejecuta `make TARGET_WEB=1`.

## Script para compilar para Raspberry Pi

[Hyenadae](https://github.com/Hyenadae/) ha creado un script que ayuda a compilar el juego para Raspberry Pi. Estos son los pasos que hace el script:
  
  * Instala las dependencias;
  * Cambia VC4_DRM en la RPi de 0 a 3;
  * Cambia ajustes en la memoria de las RPis 0 y 1 para que se pueda completar la compilación;
  * Permite la instalación de un SDL2 con KMS, lo que elimina la necesidad de usar X11 y garantiza el máximo rendimiento de cualquier RPi que ejecute VC4;
  * Clona sm64pc si no encuentra los archivos necesarios;
  * Comprueba si existen los assets y la ROM base necesaria (baserom.*.z64);
  * Compila sm64pc.

El script está incluído en la rama master, pero también puede descargarse [aquí](https://raw.githubusercontent.com/sm64pc/sm64pc/master/pisetup.sh).
# Problemas conocidos
### Problemas ya conocidos:
  * La versión EU tiene bugs en los textos y no tiene audio.
  * El movimiento analógico horizontal de la cámara vuelve al estilo antiguo en el nivel Bowser in the Dark World (#72)
  * La cámara con el ratón falla cuando disparas a Mario hacia un árbol o un tubo. (#71)
  * "make: Nothing to be done for 'default'" al compilar para web. (#67)

### Estos problemas están marcados como solucionados. Por favor, contacta si sigues teniendo estos problemas.
  * El juego se llena de flags aleatorias en las builds de 64 bits para Windows
  * Hazy Maze Cave se cuelga en pantalla completa (#57)
  * La pantalla de título no tiene el cursor para manipular a Mario en pantalla completa. (#28)
  
## Parches
En la carpeta `./enhancements` hay varios archivos `patch`, que pueden aplicarse de la siguiente manera:

```
 git apply fps.patch --ignore-whitespace --reject
```
Si ocurre un rechazo, puedes buscarlo con el comando `find | grep .rej`.
Intenta resolver los rechazos a través de [wiggle](https://github.com/neilbrown/wiggle).
```
wiggle rejection.rej --replace
```
