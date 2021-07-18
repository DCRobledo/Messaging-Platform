#Practica final parte 1 y 2


##Instalación
Para compilar el proyecto es necesario descomprimir el fichero que lo aloja e ir dentro de la carpeta en cuestión
```
tar -xvf practica_final.tar
cd practica_final
```
Una vez nos encontremos dentro podemos compilar ejecutando el comando `make [-s]` que compilará todas las dependencias y componentes

##Ejecución

###Despliegue servicio RPC
Se recomienda que este sea el primer paso a realizar, aunque no es obligatorio. Para ponerlo a ejecutar basta con escribir el comando: `./server_rpc/bin/server_rpc` y anotar el host donde se ha lanzado

###Despliegue servidor web y servicio web
Para realizar una correcta ejecución es necesario que el servicio java se encuentre en el mismo docker que el servicio web.
Para poner ambos en funcionamiento basta con ejectuar el scritp proporcionado launch\_server.sh indicando el puerto en el que se quiera empezar a escuchar
```
./launch_server.sh  <port to listen> <rpc server dir>
```

###Ejecución de los clientes
Para lanzar un cliente se ha proporcionado otro script de forma que se pueda automatizar su ejecución. Para ello solo hace falta ejecutar 
```
./client.sh <server dir> <server port>
```

##Ejecución manual
Alternativamente, a los scripts se puede ejecutar de forma manual todo el proceso de forma que resulte más transparente.

### Ejecución servidor web
Para la ejecución del servidor web basta con acceder a la carpeta donde se encuentra y ejecutar en background el servicio
```
cd WebService/textConversor/bin
java textConversor.Publisher &
```
Una vez ejecutado el servicio podemos lanzar el servidor web yendo a la carpeta raiz y lanzando el servidor
```
cd ../../../
server/bin/server -p <puerto_escucha> -s <servidor_rpc>
```

###Ejecución de los clientes
Para la ejecución de los clientes basta con ejecutar
```
java -cp client/bin:client/src:WebService client -s <server> -p <port>
```
Donde server representa el nombre del host donde se ejecutan tanto el servidor como el webservice
