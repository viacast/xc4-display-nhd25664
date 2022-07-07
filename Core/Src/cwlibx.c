/*
 * cwlinux_lib.c
 *
 *  Created on: Jan 17, 2022
 *      Author: hugin
 *      o intuito desta biblioteca é integrar os comandos reconhecidos pelo cwluinux com os definidos na biblioteca de manipulação do display
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "main.h"
#include "u8g2.h"
#include "ac.h"
#include "logo_defs.h"
#include "cwlibx.h"
#include "com_monitor.h"
#include "flash_manipulation.h"

extern u8g2_t u8g2;
extern TIM_HandleTypeDef htim1;

custom_character_t custom_character_db[CUSTOM_CHARACTER_BUFFER_SIZE];
volatile gpio_db s_pins[4];

/**@brief	Exponentiate [value] , at the [pot] level
 * eu redefini a funcão de exponenciação manualmente por não poder importar a biblioteca matematica inteira apenas por uma função :D
 *
 */
uint16_t exp(uint16_t val, uint16_t pot) {
	uint16_t ans = 1;
	while (pot) {
		ans *= val;
		pot--;
	}
	return ans;
}
void gpio_custom_init(void) {
	/* 0  ,   1 ,   2 ,  3  */
	/*PB12, PB13, PB14, PB15*/
	s_pins[0].GPIOx = GPIOB;
	s_pins[0].pin = GPIO_PIN_12;
	s_pins[0].is_init = false;
	s_pins[0].dir = '?';

	s_pins[1].GPIOx = GPIOB;
	s_pins[1].pin = GPIO_PIN_13;
	s_pins[1].is_init = false;
	s_pins[1].dir = '?';

	s_pins[2].GPIOx = GPIOB;
	s_pins[2].pin = GPIO_PIN_14;
	s_pins[2].is_init = false;
	s_pins[2].dir = '?';

	s_pins[3].GPIOx = GPIOB;
	s_pins[3].pin = GPIO_PIN_15;
	s_pins[3].is_init = false;
	s_pins[3].dir = '?';
}

void init_custom_gpio_ports(GPIO_TypeDef *GPIOx, uint16_t pin, uint8_t mode,
		uint8_t pull) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	__HAL_RCC_GPIOC_CLK_ENABLE();

	if (mode == GPIO_MODE_OUTPUT_PP) {
		/*output */
		GPIO_InitStruct.Pin = pin;
		GPIO_InitStruct.Mode = mode;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	} else {
		/*imput */
		GPIO_InitStruct.Pin = pin;
		GPIO_InitStruct.Mode = mode;
	}
	GPIO_InitStruct.Pull = pull;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}
void gpio_handler(uint8_t function, uint16_t pin) {

	uint8_t mode;
	int8_t pull;

//is init
	if (!s_pins[pin].is_init) {
		if (function == LCD_GPO_ON) {
			mode = GPIO_MODE_OUTPUT_PP;
			pull = GPIO_NOPULL;
			s_pins[pin].is_init = true;

			init_custom_gpio_ports(s_pins[pin].GPIOx, s_pins[pin].pin,
			GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
			HAL_GPIO_WritePin(s_pins[pin].GPIOx, s_pins[pin].pin, RESET);
		}
		if (function == LCD_GPO_OFF) {
			mode = GPIO_MODE_OUTPUT_PP;
			pull = GPIO_NOPULL;
			s_pins[pin].is_init = true;

			init_custom_gpio_ports(s_pins[pin].GPIOx, s_pins[pin].pin,
			GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
			HAL_GPIO_WritePin(s_pins[pin].GPIOx, s_pins[pin].pin, SET);
		}
		if (function == LCD_READ_GPI) {
			mode = GPIO_MODE_INPUT;
			pull = GPIO_NOPULL;
			s_pins[pin].is_init = true;

			init_custom_gpio_ports(s_pins[pin].GPIOx, s_pins[pin].pin,
			GPIO_MODE_INPUT, GPIO_NOPULL);
			if ((HAL_GPIO_ReadPin(s_pins[pin].GPIOx, s_pins[pin].pin))
					== GPIO_PIN_RESET) {
				monitor_send_string("1");
			} else {
				monitor_send_string("0");
			}
		}
	} else {
		if (function == LCD_GPO_ON) {
			HAL_GPIO_WritePin(s_pins[pin].GPIOx, s_pins[pin].pin, RESET);
		}
		if (function == LCD_GPO_OFF) {
			HAL_GPIO_WritePin(s_pins[pin].GPIOx, s_pins[pin].pin, SET);
		}
		if (function == LCD_READ_GPI) {
			if ((HAL_GPIO_ReadPin(s_pins[pin].GPIOx, s_pins[pin].pin))
					== GPIO_PIN_RESET) {
				monitor_send_string("0");
			} else {
				monitor_send_string("1");
			}
			s_pins[pin].is_init = false;
		}
	}
}

/**@brief	Decode Convert  Expand Rotate Encode
 * Esta função decodifica um caracter em hexadecimal dado, amplifica ele para o tamanho da memoria do novo display, rotaciona ele e recodifica
 * de maneira que no banco de dados eu só vou salvar o caractere definido pelo usuario em hexadecimal e o indice
 *
 */
void decode_convert_expand_encode(uint8_t *hex_char) {
	uint8_t mat[N][N] = { 0 };
	uint8_t i, j, k, hex, temp;

//decode_expand
	for (i = 3; *(hex_char + i) != 0xFD; i++) {
		hex = *(hex_char + i);

		for (j = 7; j >= 0 && j < 8; j--) {
			mat[i - 3][j] = hex % 2;
			hex = hex / 2;
		}
	}
//rotate
	// Consider all squares one by one
	for (i = 0; i < N / 2; i++) {
		// Consider elements in group
		// of 4 in current square
		for (j = i; j < N - i - 1; j++) {
			// Store current cell in
			// temp variable
			temp = mat[i][j];

			// Move values from right to top
			mat[i][j] = mat[j][N - 1 - i];

			// Move values from bottom to right
			mat[j][N - 1 - i] = mat[N - 1 - i][N - 1 - j];

			// Move values from left to bottom
			mat[N - 1 - i][N - 1 - j] = mat[N - 1 - j][i];

			// Assign temp to left
			mat[N - 1 - j][i] = temp;
		}
	}
//encode
	for (i = 0, k = 0; i < N; i++) {
		temp = 0;
		for (j = 0; j < N; j++) {
			temp += exp(2, N - j) * mat[i][j];
		}
		//save on the database
		if (i > 1 && k < CUSTOM_CHARACTER_SIZE) {
			hex_char[k + 3] = temp;
			k++;
		}
	}
}

/**@brief	Initialize the custom characters Database					(Default: OFF)
 *	inicializa o banco de caracteres com o que foi salvo na memoria do equipamento
 *	por hora ele seta todos os valores salvos para zero mas pode um dia buscar na
 *	memoria persistente pelos valores gravados
 *
 */
void init_custom_character_db(void) {
	memset(custom_character_db, 0, sizeof(custom_character_db));
}
// 				Command Summary
//	->TEXT Commands

/**@brief	auto line wrap												(Default: OFF)
 *	habilita a quebra de linha
 *	-on
 *	quebra de linha ativada determina que ao chegar ao fim do display o texto
 *	se quebra para a primeira posição da proxima linha
 *		FE 43 FD
 *		254 67 253
 *		254 `C` 253
 *	-off
 *	quebra de linha desativada determina que ao chegar ao fim do display o texto
 *	se quebra para a primeira posição da propria linha em que ele estava sendo escrito
 *		FE 44 FD
 *		254 68 253
 *		254 `D` 253
 */
void auto_line_wrap(bool enable) {
	//u8g2_SendF(&u8g2, "c", (enable ? 0x0a7 : 0x0a6));
	text_wrap = enable;
}

/**@brief	auto scroll 												(Default: OFF)
 *	desloca t0do o display em uma linha para abrir espaço para a ultima linha
 *	-on
 *	habilita
 *		FE 51 FD
 *		254 81 253
 *		254 `Q` 253
 *	-off
 *	desabilita
 *		FE 52 FD
 *		254 82 253
 *		254 `R` 253
 */
void auto_scroll(uint8_t enable) {
	u8g2_SendF(&u8g2, "c", 0x040);
}

/**@brief Set text insertion point									(Default: N/A)
 *	determina o ponto de inserção de texto como sendo o passado pelo usuario
 *		FE 47 [col] [row] FD
 *		254 71 [col] [row] 253
 *		254 `G` [col] [row] 253
 *	-HOME
 *	determina o ponto como sendo o inicio (0,0) do display
 *		FE 48 FD
 *		254 72 253
 *		254 `H` 253
 */
void text_insertion_point(uint8_t col, uint8_t row) {
	//convertendo a entrada para posição em termos de caractere
	cursor[0] = col * u8g2_GetMaxCharWidth(&u8g2) /*- '0'*/;
	cursor[1] = row
			* (u8g2_GetMaxCharHeight(&u8g2) - ESP_ENTRE_LINHAS)/*- '0'*/;
}

/**@brief Underline cursor 											(Default: OFF)
 *	cursor de indicação de entrada de texto
 *	-on
 *	habilita o cursor e o posiciona na coordenada determinada pelo usuario
 *		FE 4A [col] [row] FD
 *		254 74 [col] [row] 253
 *		254 `J` [col] [row] 253
 *	-off
 *	desabilita o cursor
 *		FE 4B FD
 *		254 75 253
 *		254 `K` 253
 *
 *		vars:
 *		>enable habilita o cursor na tela na dada posição (true imprime, false apaga);
 *		>col :  coluna do display onde o cursor deve ser posicionado, onde coluna é relativa ao display dividido em unidades de caractere
 *		>row :  linha na qual o cursor deve ser posicionado considerando row como a altura do displau em função de unidades de caractere
 *		>hoover :  variavel de deslocamento, pode ser 0, quando não há deslocamento, R para deslocamento de uma unidade para a direita e
 *	L para deslo camento de uma unidade para a esquerda
 *
 *
 *	Bug conhecido/possivel
 *	caso o usuario utiluze a função de apagar o cursor, sem antes telo escrito, o codigo vai de fato criar um cursor em uma posição lixo
 *	de memoria tanto para linha quanto para coluna
 */

void put_Ucursor(bool enable, uint8_t col, uint8_t row, uint8_t hoover) {
	static uint8_t ucursor[2];
	u8g2_SetDrawColor(&u8g2, 2);
	if (enable) {
		if (hoover) {
			if (hoover == 'R') {
				u8g2_DrawFrame(&u8g2, ucursor[0], ucursor[1],
						u8g2_GetMaxCharWidth(&u8g2), 1);
				ucursor[0] =
						(ucursor[0] + u8g2_GetMaxCharWidth(&u8g2))
								> u8g2_GetDisplayWidth(&u8g2)
										- u8g2_GetMaxCharWidth(&u8g2) ?
								0 : ucursor[0] + u8g2_GetMaxCharWidth(&u8g2);
				u8g2_DrawFrame(&u8g2, ucursor[0], ucursor[1],
						u8g2_GetMaxCharWidth(&u8g2), 1);
			} else {
				u8g2_DrawFrame(&u8g2, ucursor[0], ucursor[1],
						u8g2_GetMaxCharWidth(&u8g2), 1);
				ucursor[0] =
						(ucursor[0] - u8g2_GetMaxCharWidth(&u8g2)) < 0 ?
								u8g2_GetDisplayWidth(
										&u8g2) - u8g2_GetMaxCharWidth(&u8g2) :
								ucursor[0] - u8g2_GetMaxCharWidth(&u8g2);
				u8g2_DrawFrame(&u8g2, ucursor[0], ucursor[1],
						u8g2_GetMaxCharWidth(&u8g2), 1);
			}
		} else {
			ucursor[0] = col * u8g2_GetMaxCharWidth(&u8g2);
			ucursor[1] = (row + 1) * u8g2_GetMaxCharWidth(&u8g2) + 2;
			u8g2_DrawFrame(&u8g2, ucursor[0], ucursor[1],
					u8g2_GetMaxCharWidth(&u8g2), 1);
		}
	} else {
		u8g2_DrawFrame(&u8g2, ucursor[0], ucursor[1],
				u8g2_GetMaxCharWidth(&u8g2), 1);
	}
	u8g2_SendBuffer(&u8g2);
	u8g2_SetDrawColor(&u8g2, 1);
}

/**@brief Inverse Text												(Default: OFF)
 *	comando de inversão de texto, o fundo do texto passa a ser de pixels ativos e o texto
 *	passa a ser pixels inativos
 *	-on
 *	ativa a inversão de texto
 *		FE 66 FD
 *		254 102 253
 *		254 `f` 253
 *	-off
 *	desativa a inversão de texto
 *		FE 67 FD
 *		254 103 253
 *		254 `g` 253
 */
void inverse_text(bool state) {
	text_invertion = state;
}

//	->Bar Charts and Graphic Commands

/**@brief Initialize thick vertical bar graph							(Default: ON)
 *	Define a barra vertical como com 5px de largura
 *	FE 76 FD
 *	254 118 253
 *	254 `v` 253
 */
/**@brief Inithialize thin vertical bar graph							(Default: OFF)
 * 	Define a barra vertical como com 2px de largura
 *	FE 73 FD
 *	254 115 253
 *	254 `s` 253
 */
void def_v_bar_thickness(uint8_t thick) {
	if (thick) {
		vertical_bar_width = 10;			//5 no original
	} else {
		vertical_bar_width = 4;			//2 no original
	}
}

/**@brief Define a custom caracter									(Default: N/A)
 *	Define um caracter customizado, [cc] varia de [[0x01] a [0x10]].
 *	Os 6 bytes variam da seguinte forma:
 *	FE 4E [cc] [6bytes] FD
 *	254 104 [cc] [6bytes] 253
 *	254 `N` [cc] [6bytes] 253
 */
void define_custom_character(uint8_t *cmd) {
	decode_convert_expand_encode(cmd);
	memcpy(custom_character_db[*(cmd + 2)].custom_caracter, cmd + 3, 6);
	custom_character_db[*(cmd + 2)].custo_character_index = *(cmd + 2);
}

/**@brief Draw a vertical bar graph									(Default: N/A)
 *	Desenha uma coluna vertical na ultima linha da coluna [col] de altura [height], com [height]
 *	variando de [[0x00] a [0x7A]] (0 a 122)
 *	FE 3D [col] [height] FD
 *	254 61 [col] [height] 253
 *	254 `=` [col] [height] 253
 */
void draw_un_v_bar_graph(uint8_t col, uint8_t height, bool erase) {
	static uint8_t vcursor, temp_height = 200, temp_col = 200;

	vcursor = col * u8g2_GetMaxCharWidth(&u8g2);
	height = u8g2_GetDisplayHeight(&u8g2) - height * 2;

	if (!erase) {
		u8g2_SetDrawColor(&u8g2, 0);
		u8g2_DrawBox(&u8g2, temp_col, temp_height, vertical_bar_width,
				u8g2_GetDisplayHeight(&u8g2));

		u8g2_DrawBox(&u8g2, vcursor, height, vertical_bar_width,
				u8g2_GetDisplayHeight(&u8g2));

		temp_height = height;
		temp_col = vcursor;

	} else {
		u8g2_SetDrawColor(&u8g2, 0);
		u8g2_DrawBox(&u8g2, vcursor, height, vertical_bar_width,
				u8g2_GetDisplayHeight(&u8g2));
	}
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_SendBuffer(&u8g2);

}

/**@brief Erase a horizontal bar graph									(Default: N/A)
 *	Apaga a barra horizontal na coluna [col] da linha [row] com largura [length]
 *	onde length varia [[0x00] to [0x7A]] (0 a 122)
 *	FE 2D [col] [row] FD
 *	254 45 [col] [row] 253
 *	254 `-` [col] [row] 253
 */
/**@brief Draw a horizontal bar graph									(Default: N/A)
 *	Desenha uma coluna vertical na ultima linha da coluna [col] de altura [height], com [height]
 *	variando de [[0x00] a [0x7A]] (0 a 122)
 *	FE 7C [cc] [height] FD
 *	254 124 [cc] [height] 253
 *	254 `|` [cc] [height] 253
 */
void draw_un_h_bar_graph(uint8_t col, uint8_t row, uint8_t lenght, bool erase) {
	if (!erase) {
		u8g2_DrawBox(&u8g2, (col) * u8g2_GetMaxCharWidth(&u8g2),
				(row) * u8g2_GetMaxCharHeight(&u8g2), lenght * 2,
				u8g2_GetMaxCharHeight(&u8g2) - 3);

	} else {
		u8g2_SetDrawColor(&u8g2, 0);
		u8g2_DrawBox(&u8g2, (col) * u8g2_GetMaxCharWidth(&u8g2),
				(row) * u8g2_GetMaxCharHeight(&u8g2), lenght * 2,
				u8g2_GetMaxCharHeight(&u8g2) - 3);

	}
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_SendBuffer(&u8g2);
}

/**@brief Put pixel													(Default: N/A)
 *	Desenha um pixel na posição (x,y). X varia de 0-121 e y varia 0-31
 * 	FE 70 [x] [y] FD
 *	254 112 [x] [y] 253
 *	254 `p` [x] [y] 253
 */
void put_pixel(uint8_t x, uint8_t y) {
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_DrawPixel(&u8g2, x, y);
	u8g2_SendBuffer(&u8g2);
}

/**@brief Clear Pixel													(Default: N/A)
 *	Apaga um pixel na posição (x,y). X varia de 0-121 e y varia 0-31
 *	FE 71 [x] [y] FD
 *	254 113 [x] [y] 253
 *	254 `q` [x] [y] 253
 */
void erase_pixel(uint8_t x, uint8_t y) {
	u8g2_SetDrawColor(&u8g2, 0);
	u8g2_DrawPixel(&u8g2, x, y);
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_SendBuffer(&u8g2);
}

/** @brief Draw a byte													(Default: N/A)
 *	Desenha um byte na posição [x,row] onde x varia de 0-121 e y varia de 0-3
 *	FE 3E [x] [row] [byte] [4 dummy bytes] FD
 *	254 62 [x] [row] [byte] [4 dummy bytes]  253
 *	254 `>` [x] [row] [byte] [4 dummy bytes]  253
 */
void put_byte(uint8_t x, uint8_t row, uint8_t byte) {
	char arr[8], i;
	memset(arr, 0, 8);
	for (i = 0; byte > 0; i++) {
		arr[i] = byte % 2;
		byte = byte / 2;
	}
	row = row * ((u8g2_GetMaxCharHeight(&u8g2)) - ESP_ENTRE_LINHAS);
	u8g2_DrawXBM(&u8g2, x, row, 1, 8, arr);
	u8g2_SendBuffer(&u8g2);

}

// Miscellaneous command summary

/** @brief reseta por software o display
 *   	FE 56 FD
 *		254 86 253
 *		254 'V' 253
 *	@retval
 */

void lcd_soft_reset(void) {
	HAL_NVIC_SystemReset();
}

/** @brief Clear Display																(Default: N/A)
 * Limpa a tela e posiciona o cursor no canto superior esquerdo
 *   	FE 58 FD
 *		254 88 253
 *		254 'X' 253
 *	@retval  flag confirmando que o comando foi executado
 */
void clear_display(void) {
	text_insertion_point(0, 0);
	u8g2_ClearDisplay(&u8g2);
}

/** @brief Backlight state																(Default: N/A)
 * Define o status da Backlight/ Luz de fundo do display
 *  -on
 *  Habilita o Backlight
 *   	FE 42 FD
 *		254 66 253
 *		254 'B' 253
 *  -off
 *  Desabilita o Backlight
 *   	FE 46 FD
 *		254 70 253
 *		254 'F' 253
 */
void enable_backlight(bool enable) {
	//liga a backlight
	if (enable) {
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, LCD_BRIGHT);
	} else {
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
	}
}

/** @brief Backlight brightness															(Default: N/A)
 * Define o brlho do backlight em um dos niveis pre-configurados, [bright] varia de 1-7
 *   	FE 41 [bright] FD
 *		254 64 [bright] 253
 *		254 'A' [bright] 253
 */
void set_backlight_brightness(uint8_t bright) {
	if (bright) {
		LCD_BRIGHT = (7 / bright) * 59999;
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, LCD_BRIGHT);
	}
}

/** @brief Auto Key Hold state															(Default: N/A)
 * -on
 * Habilita a função key hold
 *   	FE 32 FD
 *		254 50 253
 *		254 '2' 253
 * -off
 * desabilita a função key hold
 *   	FE 33 FD
 *		254 51 253
 *		254 '3' 253
 */
void key_hold_state(uint8_t state);

/** @brief Set RS232 port speed															(Default: 19200)
 * Define a velocidade da comunicação RS232-ttl. As velocidades são tabeladas. A saber:
 * [speed] esta contido no intervalo {1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 230400}
 *   	FE 39 [speed] FD
 *		254 57 [speed] 253
 *		254 '9' [speed] 253
 */
void set_rs232_speed(uint8_t speed);

/** @brief Save user defined characters													(Default: N/A)
 * Salva um caractere definido pelo usuario. [cc] varia de 1-16
 *   	FE 4F [cc] FD
 *		254 79 [cc] 253
 *		254 'O' [cc] 253
 */
void save_user_character(uint8_t cc);

/** @brief Load user defined characters													(Default: N/A)
 * Carrega um caractere definido pelo usuario. [cc] varia de 1-16
 *   	FE 50 [cc] FD
 *		254 90 [cc] 253
 *		254 'P' [cc] 253
 */
void load_user_characters(uint8_t cc);

/** @brief Save user settings															(Default: N/A)
 * Salva os [4bytes] de configurações definidas pelo usuario [ud], onde [ud] varia de 1-8
 *   	FE 53 [ud] [4bytes] [2dummy_bytes] FD
 *		254 83 [ud] [4bytes] [2dummy_bytes] 253
 *		254 'S' [ud] [4bytes] [2dummy_bytes] 253
 */
uint8_t save_user_settings(uint8_t ud, uint8_t *settings);

/** @brief Load user settings															(Default: N/A)
 * Carrega da memoria as configurações definidas pelo usuario indicado por [ud]
 *   	FE 54 [ud] FD
 *		254 84 [ud] 253
 *		254 'T' [ud] 253
 */
uint8_t load_user_settings(uint8_t ud);

/** @brief Relay state																	(Default: N/A)
 * Ativa o relay
 * -on
 * Ele será sempre ativo se o [timeout] = 0, entretanto ele será ativo por [timeout] segundos
 * sempre que [timeout] > 0. [timeout] é um numero inteiro contido em {0,...,10}
 *   	FE 61 [timeout] FD
 *		254 97 [timeout] 253
 *		254 'a' [timeout] 253
 * -off
 * Desativa o Relay
 *   	FE 62 FD
 *		254 98 253
 *		254 'b' 253
 */
uint8_t relay_set_state(uint8_t timeout);

/** @brief GPO state																	(Default: N/A)
 * define o status da porta gpio [gpo#]
 * -on
 * define a porta como High
 *   	FE 63 [gpi#] FD
 *		254 99 [gpi#] 253
 *		254 'c' [gpi#] 253
 * -off
 * define a porta como Low
 *   	FE 64 [gpi#] FD
 *		254 100 [gpi#] 253
 *		254 'd' [gpi#] 253
 */
uint8_t gpo_set_state(uint8_t gpo, uint8_t status);

/** @brief GPI Status																	(Default: N/A)
 * Retorna o valor de status da porta gpio [gpi#] definida como entrada na forma de bit
 *   	FE 65 [gpi#] FD
 *		254 101 [gpi#] 253
 *		254 'e' [gpi#] 253
 *	@retval flag indicando que deu certo [bool]
 */
uint8_t gpi_get_state(uint8_t gpi);

/** @brief Set contrast																	(Default: N/A)
 * define o contraste da tela, range [[0x00]-[0x1C]] - 0 a 25
 *    	FE 68 FD
 *		254 104 253
 *		254 'h' 253
 */
void set_contrast(uint8_t contrast) {
	u8g2_SetContrast(&u8g2, (contrast) * 9);
}

/** @brief Save Boot-up Logo															(Default: N/A)
 * Salva a tela atual como splash/boot-up screen
 *  	FE 69 FD
 *		254 106 253
 *		254 'i' 253
 */
void save_screen_to_splash(void);

/** @brief Display Boot-up Logo															(Default: N/A)
 * Desenha a splash screen/boot-up na tela imediatamente
 *  	FE 69 FD
 *		254 106 253
 *		254 'j' 253
 */
void disp_splash(void) {
	u8g2_DrawXBM(&u8g2, ((display_w / 2) - (vc_width / 2)),
			((display_h / 2) - (vc_height / 2)), vc_width, vc_height, &vc_logo_bits);
	u8g2_SendBuffer(&u8g2);

}

/** @brief Restore default factory logo													(Default: N/A)
 * Restaura a splash screen/boot-up definida pela fabrica
 * 		FE 6B FD
 * 		254 107 253
 * 		254 'k' 253
 */
void restore_def_logo(void);

/** @brief write a string on screen														(Default: N/A)
 * escreve um array na tela.
 *
 */
void str_warper(txt_wrap_t *wrap, uint8_t *txt) {
	uint8_t index = 0, aux, delta;
	uint8_t shift = 0, b_spaces;
	uint8_t bgn_offset = 0;

	clean_it(wrap->wrap_str);

	index = strlen(txt);

	//considerar espaços brancos na primeira linha pra divisão entrar em modo append
	b_spaces = cursor[0] / u8g2_GetMaxCharWidth(&u8g2);
	index += b_spaces;

	//calcular o numero de linhas necessarias
	wrap->wrap_times = index / MAX_CHARS_ONSCREEN;

	if (wrap->wrap_times) {
		for (aux = 0; aux <= wrap->wrap_times; aux++) {
			if (aux < N_LINES) {

				delta = MAX_CHARS_ONSCREEN - (aux == 0 ? b_spaces : 0);

				strncpy(wrap->wrap_str[aux], txt + ((bgn_offset) * (aux)), delta);

				wrap->wrap_str[aux][delta] = '\0';
				bgn_offset = delta;

			} else {
				shift = *(txt + ((MAX_CHARS_ONSCREEN - 1) * (aux))) == ' ' ?
						0 : 1;
				strncpy(wrap->wrap_str[aux],
						txt + ((MAX_CHARS_ONSCREEN) * (aux)) + shift,
						strlen(txt + ((MAX_CHARS_ONSCREEN - 1) * (aux))));
			}
		}
	}

}

bool Custom_Character_masker(uint8_t *txt, uint8_t *mask, uint8_t *v_cursor) {
	uint8_t index;
	bool enable;
	strncpy(mask, txt, MASK_BUFFER);
	*(mask + MASK_BUFFER - 1) = '\0';

	*(v_cursor + 0) = cursor[0];
	*(v_cursor + 1) = cursor[1];

	for (index = 0; *(mask + index) != '\0'; index++) {
		if (*(mask + index) >= 0 && *(mask + index) <= 16) {
			*(mask + index) = 32;
			enable = true;
		}
	}
	return enable;
}
void clean_it(uint8_t *str) {
	memset(str, 0, sizeof(str));
}
void custom_character_dealer(uint8_t *txt) {
	uint8_t char_w = 8;
	uint8_t char_h = 6;
	uint8_t offset, temp_x, temp_y, index;
	temp_x = cursor[0];
	temp_y = cursor[1];
	for (offset = 0; *(txt + offset) != '\0'; offset++) {
		index = *(txt + offset);
		if (index <= 16 && (custom_character_db[index].custo_character_index)) {
			u8g2_DrawXBM(&u8g2, temp_x, temp_y + 4, char_w, char_h,
					custom_character_db[index].custom_caracter);
			u8g2_SendBuffer(&u8g2);
			temp_x += u8g2_GetMaxCharWidth(&u8g2);
			//trocando os caracteres especiais por um espaço em branco pra bater com a conta de espaço em tela
			//é um custom caracter
			//ele foi definido pelo usuario
			//ele deve ser impresso
		} else {
			temp_x += u8g2_GetMaxCharWidth(&u8g2);
		}
	}

}

void lcd_print(uint8_t *txt) {
	txt_wrap_t wrap;
	uint8_t aux = 0;
	static uint8_t v_cursor[2];
	uint8_t mask[MASK_BUFFER];
	bool enable;

	if (text_invertion) {
		if (text_wrap) {
			str_warper(&wrap, txt);
			for (aux = 0; aux <= wrap.wrap_times; aux++) {

				u8g2_DrawButtonUTF8(&u8g2, cursor[0], cursor[1], U8G2_BTN_INV,
						0, 0, 0, wrap.wrap_str[aux]);
				u8g2_SendBuffer(&u8g2);
			}
			clean_it(wrap.wrap_str);
		} else {

			u8g2_DrawButtonUTF8(&u8g2, cursor[0], cursor[1], U8G2_BTN_INV, 0, 0,
					0, txt);
			u8g2_SendBuffer(&u8g2);

			cursor[0] += u8g2_GetStrWidth(&u8g2, txt);
		}

	} else {
		if (text_wrap) {
			str_warper(&wrap, txt);
			if (wrap.wrap_times) {
				for (aux = 0; aux <= wrap.wrap_times; aux++) {
					clean_it(mask);
					if (aux) {
						cursor[1] += (u8g2_GetMaxCharHeight(&u8g2))
								- ESP_ENTRE_LINHAS;
						v_cursor[1] = cursor[1];
						cursor[0] = 0;
					}
					enable = Custom_Character_masker(wrap.wrap_str[aux], mask,
							v_cursor);
//					u8g2_DrawUTF8(&u8g2, cursor[0], cursor[1],wrap.wrap_str[aux]);
//					u8g2_SetDrawColor(&u8g2, 2);

					u8g2_DrawUTF8(&u8g2, v_cursor[0], v_cursor[1], mask);

					if (enable) {
						custom_character_dealer(wrap.wrap_str[aux]);
					}

					u8g2_SendBuffer(&u8g2);
					u8g2_SetDrawColor(&u8g2, 1);
				}
				cursor[0] += u8g2_GetStrWidth(&u8g2, mask);
				clean_it(wrap.wrap_str);
			} else {

				u8g2_DrawUTF8(&u8g2, cursor[0], cursor[1], txt);
				cursor[0] += u8g2_GetStrWidth(&u8g2, txt);
				u8g2_SendBuffer(&u8g2);
			}

			/*if (u8g2_GetStrWidth(&u8g2,
			 wrap.wrap_str[aux - 1]) > u8g2_GetDisplayWidth(&u8g2)) {
			 cursor[1] = (aux * u8g2_GetMaxCharHeight(&u8g2));
			 cursor[0] = 0;
			 } else {
			 cursor[0] += u8g2_GetStrWidth(&u8g2, wrap.wrap_str[aux - 1]);
			 }*/
		} else {
			enable = Custom_Character_masker(txt, mask, v_cursor);
			u8g2_DrawUTF8(&u8g2, v_cursor[0], v_cursor[1], mask);

			if (enable) {
				custom_character_dealer(txt);
			}
			cursor[0] += u8g2_GetStrWidth(&u8g2, txt);
		}
	}
	u8g2_SendBuffer(&u8g2);
}

