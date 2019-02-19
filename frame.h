
void	send_frame(uint8_t *frame, int size);
void	send_byte(uint8_t ch);

int 	get_frame(uint8_t *buf);
int		get_command();
uint8_t	*get_data(int arg, int size);
uint8_t	 get_mode();
char   *get_str(int arg);
int		get_4byte(int arg);
int		get_intvalue(int arg);
float	get_floatvalue(int arg);

uint8_t	get_mode();
int		get_4byte2(int arg);		
int		get_intvalue2(int arg);
char   *get_string2(int arg);


int	get_byte(uint8_t *data);

