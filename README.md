# Experiencia Laborativa Número 2

A continuación se pormenoriza una superflua y para nada innecesaria serie de pasos para perpetrar el proyecto en un entorno de consola de Linux:

Ci-après est pormenorisée une superflue et majestueuse série d'étapes pour perpétrer le projet dans un environnement de console Linux.

======

-> Importante tener la version 5.5v de idf

-> Verificar conectividad de la placa: 
ls /dev/ttyUSB*

-> Cargar entorno:
. ~/esp/esp-idf/export.sh

-> Verificar que esta cargado:
$IDF_PATH

-> Targetear el esp32:
idf.py set-target esp32s2

-> Flashear la placa:
idf.py -p /dev/ttyUSB* flash monitor (podria ser USB1 - USB0, se tendria que revisar particularmente)

-- EN CASO DE NECESITARLO --
-> Limpiar basura: 
idf.py fullclean






