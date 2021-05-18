# Trabalho Final 2020/2

[Descrição do projeto](https://gitlab.com/fse_fga/projetos_2020_2/trabalho-final-2020-2)

## Identificação
- Aluno: Guilherme Guy de Andrade
- Matrícula: 16/0123186

## Explicação geral do projeto

O projeto foi dividido em 2 modulos:

- Servidor em Python para gerenciar os clientes e prover interface grafica ao usuário
- Código em c que é executado na ESP32

Além disso, elaborei um simulador simples, que me auxiliou a iniciar o desenvolvimento do servidor emulando algumas das mensagens MQTT que a ESP32 deveria enviar.


## Passos para execução

### Servidor Python
- Recomendo iniciar uma venv para instalar os requisitos do Python
- Instalar os requisitos com `pip install -r requirements.txt`
- Executar `make run-server` que irá executar os comnados necessários para subir o servidor
- Após isso o servidor estará disponível em [http://localhost:8000](http://localhost:8000)

### ESP32 - Energia
- Com o ambiente ESP-IDF configurado
- Execute `idf.py menuconfig` para acessar os menus "Wifi Configuration" e "MQTT Configuration" e configurar as opções de Wifi (SSID e senha) e o host MQTT (opcional), respectivamente
- Garantir que o módulo DHT11 esteja conectado a GPIO_4 da ESP e devidamente alimentado
- Compilar com `idf.py build`
- Realizar o flash e monitoramento da placa com `idf.py -p <PORTA> flash monitor`
- Obs: pode ser necessário limpar a memoria da placa utilizando `idf.py -p <PORTA> erase_flash`

### ESP32 - Bateria
- Infelizmente, não está presente

## Controles do programa

### Servidor

[](docs/ui_img_1.png)

Esta é a tela principal do programa. A área em vermelho mostra dispositivos que se identificaram, mas ainda necessitam do cadastro do usuário. 

A área em amarelo mostra todas as localidades e seus dispositivos, além de exibir as informações de temperatura e humidade para cada dispositivo e também uma média de todos os dispositivos em determinada localidade.

Ao clicar na área em verde o usuário irá retornar para a página inicial.

[](docs/ui_img_2.png)

Na hora de cadastrar um dispositivo é necessário nomear suas entradas e saídas, além de definir uma localidade. Caso a localidade não exista, ela será criada automaticamente.

[](docs/ui_img_3.png)

A tela de detalhes do dispositivo exibe as informações dele, além de suas entradas e saídas, permitindo a execução de ações nelas. Além disso é possível remover o dispositivo nesta tela.

Clicando em "Go to alarms..." é possível acessar a tela de configuração de alarmes para a entrada específica.

Ao clicar em "Change State" será enviada uma mensagem para a ESP32 trocar o estado de determinada saída, como por exemplo, ligar um LED.

[](docs/ui_img_4.png)

É possível definir alarmes para as entradas do dispositivo. A tela de alarmes exibe todos os alarmes criados para uma entrada em específico.

[](docs/ui_img_5.png)

Ao criar um alarme é possível definir um valor alvo e um método de comparação para a ativação do alarme.

[](docs/ui_img_6.png)

Caso algum alarme esteja ativo, será exibido um texto, identificando-o. Além da opção de desativar o alarme.

Um aviso sonoro também é tocado, mas em alguns navegadores é necessário interagir com o site antes que o aviso seja reproduzido.

### ESP32

Ao iniciar a placa, ela buscará se identificar no servidor e ficará aguardando o registro ser completo. Caso ela já esteja registrada irá enviar dados ao servidor e restaurará o valor das saídas GPIO. Caso o usuário tenha removido a placa no servidor, ela retornará aos dados iniciais.

A GPIO_O está configurada como uma das entradas aceitas no servidor. Caso um alarme esteja configurado apertar este botão irá acioná-lo. Segurar o botão da GPIO_0  por 5 segundos irá reiniciar a placa para as configurações padrão. Atenção: caso o servidor ainda possua o registro da placa, ela será automaticamente registrada novamente. 

## Demonstração em vídeo

TODO