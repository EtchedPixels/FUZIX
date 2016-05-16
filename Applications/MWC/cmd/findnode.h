
typedef	struct	NODE	{
	struct	NODE	*n_left;
	struct	NODE	*n_right;
	union {
		unsigned n_val;
		char	*n_str;
		char	**n_strp;
		time_t	n_time;
	}	n_un;
	int	(*n_fun)(struct NODE *);
	int	n_op;
	int	n_type;
}	NODE;
