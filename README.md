Objetivo

  Implementar el juego del gato con hilos múltiples, para varias partidas simultaneas en lenguaje C.
  
Características

  •La aplicación será en línea de comandos.
  
  •El servidor esperará conexiones de clientes TCP y los pondrá a jugar en parejas conforme se vayan conectando.
  
  •Mediante control de concurrencia, el servidor permitirá alternar la tirada entre jugadores por cada partida.
  
  •El primer jugador conectado será las X y el segundo las O y jugaran en dicho orden.
  
  •Por cada tirada de jugador se actualizará el estado del juego de gato mostrado en la línea de comandos.

  Aplicación:
  
• Cada partida corresponderá a un hilo y la conexión de dos clientes.

• Cada hilo controlará una partida entro dos usuarios.

• Cada hilo controlará la alternancia en el turno de juego.

• El hilo se encargará de controlar que no se elija una casilla por más de un jugador.

• Una vez que alguno gane o se tenga un empate, avisara a los jugadores con un mensaje solicitando un retorno de 
  carro para terminar las conexiones y el hilo sin alterar el resto de las partidas en ejecución.
  
• Las partidas deberán de funcionar de forma simultánea y sin interacción entre ellas.






