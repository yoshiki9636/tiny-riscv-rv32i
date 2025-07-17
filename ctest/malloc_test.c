#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
//#include <math.h>
//#include <string.h>

//#define LP 10
#define LP 1000
#define LP2 200
#define NUM_NODE 4
#define NUM_EDGE 5
#define START_NODE 0
#define END_NODE 3

typedef struct _node_base t_NODE_BASE;
typedef struct _edge_base t_EDGE_BASE;
typedef struct _edge_list t_EDGE_LIST;
typedef struct _node_list t_NODE_LIST;

struct _node_base {
    int na;
    int num_e;
    int cur_dist;
    int chk_flg;
	t_EDGE_LIST* edges;
    t_NODE_LIST* cur_route;
};

struct _edge_base {
    int ea;
    int na;
    int nb;
    int dist;
};

struct _edge_list {
    int ea;
    t_EDGE_LIST* next;
};

struct _node_list {
    int na;
    t_NODE_LIST* next;
};


int __errno;

char* heap_end = (char*)0x02000000;
//void _sbrk_r(void) {}
char* _sbrk(int incr) {
 char* heap_low = (char*)0x02000000;
 char* heap_top = (char*)0x03000000;
 char *prev_heap_end;
 
 if (heap_end == 0) {
  heap_end = heap_low;
 }
 prev_heap_end = heap_end;
 
 if (heap_end + incr > heap_top) {
  /* Heap and stack collision */
  return (char *)0;
 }
 
 heap_end += incr;
 return (char*) prev_heap_end;
}


int start_dijkstra(int start_node, int end_node, int num_node, int num_edge, t_NODE_BASE* node_info, t_EDGE_BASE* edge_info);
int marking_next_node(int cur_node, t_NODE_BASE* node_info, t_EDGE_BASE* edge_info);
int node_not_finished(int num_node, t_NODE_BASE* node_info, t_EDGE_BASE* edge_info);
int init_graph(int num_node, int num_edge, int* edge_data, t_NODE_BASE* node_info, t_EDGE_BASE* edge_info);
int print_edge_info(int num_edge, t_EDGE_BASE* edge_info);
int print_node_info(int num_node, t_NODE_BASE* node_info);
int int_print( char* cbuf, int value, int type );
void uprint( char* buf, int length, int ret );
static void clearbss(void);
void pass();
void wait();

int edge_init_data[NUM_EDGE*3] = {
		0, 1, 4,
		0, 2, 2,
		1, 2, 1,
		1, 3, 3,
		2, 3, 1,
};

int main() {
	char cbuf[64];
	t_NODE_BASE node_info[NUM_NODE];
	t_EDGE_BASE edge_info[NUM_EDGE];
	//t_NODE_BASE start_node;
	//t_NODE_BASE end_node;
    unsigned int* led = (unsigned int*)0xc000fe00;

	clearbss();

	t_NODE_LIST* node_bottom;
	node_bottom = (t_NODE_LIST*)malloc(sizeof(t_NODE_LIST));
	node_bottom->na = -1;
	node_bottom->next = NULL;
	t_NODE_LIST* node_entry = node_bottom;

	t_EDGE_LIST* edge_bottom;
	edge_bottom = (t_EDGE_LIST*)malloc(sizeof(t_EDGE_LIST));
	edge_bottom->ea = -1;
	edge_bottom->next = NULL;
	t_EDGE_LIST* edge_entry = edge_bottom;

	for (int i = 0; i < 1000; i++) {
		t_NODE_LIST* node_tmp = (t_NODE_LIST*)malloc(sizeof(t_NODE_LIST));
		node_tmp->na = i+200000;
		node_tmp->next = node_entry;
		node_entry = node_tmp;

		t_EDGE_LIST* edge_tmp = (t_EDGE_LIST*)malloc(sizeof(t_EDGE_LIST));
		edge_tmp->ea = i+30000;
		edge_tmp->next = edge_entry;
		edge_entry = edge_tmp;
	}


	while ( node_entry->next != NULL) {
		int aa = node_entry->na;
		*led = aa & 0x7777;
		int length = int_print( cbuf, aa, 0 );
		uprint( cbuf, length, 2 );
		node_entry = node_entry->next;
	}

	while ( edge_entry->next != NULL) {
		int aa = edge_entry->ea;
		*led = aa & 0x7777;
		int length = int_print( cbuf, aa, 0 );
		uprint( cbuf, length, 2 );
		edge_entry = edge_entry->next;
	}
	
	pass();
	return 0;
}

/*
int node_not_finished(int num_node, t_NODE_BASE* node_info, t_EDGE_BASE* edge_info) {
	for (int i = 0; i < num_node; i++) {
		if ( node_info[i].chk_flg == 0) {
			int flg = 0;
			t_EDGE_LIST* edge = node_info[i].edges;
			int next_node;
			while ( flg == 0 ) {
				next_node = (edge_info[edge->ea].na == i) ? edge_info[edge->ea].nb : edge_info[edge->ea].na;
				if (node_info[next_node].chk_flg == 1) {
					flg = 1;
				}
				else if ( edge->next == NULL ) {
					flg = 2;
				}
				else {
					edge = edge->next;
				}
			}
			if ( flg == 1 ) {
				return i;
			}
		}
	}
	return -1; // all nodes are finished
}

int marking_next_node(int cur_node, t_NODE_BASE* node_info, t_EDGE_BASE* edge_info) {
	char cbuf[64];
	int cur_dist = node_info[cur_node].cur_dist;
	t_EDGE_LIST* edge =  node_info[cur_node].edges;
	while ( edge != NULL ) {
		int next_node = (edge_info[edge->ea].na == cur_node) ? edge_info[edge->ea].nb : edge_info[edge->ea].na;
		int length = int_print( cbuf, next_node, 0 );
		uprint( cbuf, length, 2 );
		if (node_info[next_node].chk_flg == 0) {
			int distance = cur_dist + edge_info[edge->ea].dist;
			if (distance < node_info[next_node].cur_dist) {
				// use new data
				node_info[next_node].cur_dist = distance;
				// erace old list
				t_NODE_LIST* erace_form = node_info[next_node].cur_route;
				while ( erace_form != NULL ) {
					t_NODE_LIST* next_erace_from = erace_form->next;
					free ( erace_form );
					erace_form = next_erace_from;
				}
				t_NODE_LIST* copy_from = node_info[cur_node].cur_route;
				t_NODE_LIST* copy_to = NULL;
				t_NODE_LIST* node_entry;
				while ( copy_from != NULL ) {
					node_entry = (t_NODE_LIST*)malloc(sizeof(t_NODE_LIST));
					node_entry->na = copy_from->na;
					node_entry->next = copy_to;
					copy_to = node_entry;
					copy_from = copy_from->next;
				}
				// add current node
				node_entry = (t_NODE_LIST*)malloc(sizeof(t_NODE_LIST));
				node_entry->na = cur_node;
				node_entry->next = copy_to;
				node_info[next_node].cur_route = node_entry;
			} // else higher distance
		} // else fixed node
		edge = edge->next;
	}
	// marking finished
	node_info[cur_node].chk_flg = 1;
	return 0;
}

int start_dijkstra(int start_node, int end_node, int num_node, int num_edge, t_NODE_BASE* node_info, t_EDGE_BASE* edge_info) {
	char cbuf[64];

	// set start node
	node_info[start_node].cur_dist = 0;

	// 1st marking 
	marking_next_node(start_node, node_info, edge_info);

	int cur_node = node_not_finished(num_node, node_info, edge_info);
	while ( cur_node != -1 ) {
		uprint( "current node = ", 15, 0 );
		int length = int_print( cbuf, cur_node, 0 );
		uprint( cbuf, length, 2 );

		// marking
		marking_next_node(cur_node, node_info, edge_info);
		cur_node = node_not_finished(num_node, node_info, edge_info);
	}

	return 0;
}

int init_graph(int num_node, int num_edge, int* edge_data, t_NODE_BASE* node_info, t_EDGE_BASE* edge_info) {
	t_EDGE_LIST* edge_entry;
	for (int i = 0; i < num_node; i++) {
		node_info[i].na = i;
		node_info[i].num_e = 0;
		node_info[i].cur_dist = INT_MAX;
		node_info[i].chk_flg = 0;
		node_info[i].edges = NULL;
		node_info[i].cur_route = NULL;
	}
	for (int i = 0; i < num_edge; i++) {
		edge_info[i].ea = i;
		edge_info[i].na = edge_data[i*3];
		edge_info[i].nb = edge_data[i*3+1];
		edge_info[i].dist = edge_data[i*3+2];
		// add edge to node_info
		edge_entry = (t_EDGE_LIST*)malloc(sizeof(t_EDGE_LIST));
		edge_entry->ea = i;
		node_info[edge_info[i].na].num_e++;
		edge_entry->next = node_info[edge_info[i].na].edges;
		node_info[edge_info[i].na].edges = edge_entry;
		edge_entry = (t_EDGE_LIST*)malloc(sizeof(t_EDGE_LIST));
		edge_entry->ea = i;
		node_info[edge_info[i].nb].num_e++;
		edge_entry->next = node_info[edge_info[i].nb].edges;
		node_info[edge_info[i].nb].edges = edge_entry;
	}
	return 0;
}

int print_edge_info(int num_edge, t_EDGE_BASE* edge_info) {
	char cbuf[64];
	for (int i = 0; i < num_edge; i++) {
		uprint( "edge [ ", 7, 0 );
		int length = int_print( cbuf, i, 0 );
		uprint( cbuf, length, 0 );
		uprint( "] : between ", 12, 0 );
		length = int_print( cbuf, edge_info[i].na, 0 );
		uprint( cbuf, length, 0 );
		uprint( " : ", 3, 0 );
		length = int_print( cbuf, edge_info[i].nb, 0 );
		uprint( cbuf, length, 0 );
		uprint( " : dist = ", 10, 0 );
		length = int_print( cbuf, edge_info[i].dist, 0 );
		uprint( cbuf, length, 2 );
	}
	return 0;
}

int print_node_info(int num_node, t_NODE_BASE* node_info) {
	char cbuf[64];
	for (int i = 0; i < num_node; i++) {
		uprint( "node [ ", 7, 0 );
		int length = int_print( cbuf, i, 0 );
		uprint( cbuf, length, 0 );
		uprint( "] : node_num = ", 15, 0 );
		length = int_print( cbuf, node_info[i].na, 0 );
		uprint( cbuf, length, 0 );
		uprint( " : num_edge = ", 14, 0 );
		length = int_print( cbuf, node_info[i].num_e, 0 );
		uprint( cbuf, length, 0 );
		uprint( " : cur_dist = ", 14, 0 );
		length = int_print( cbuf, node_info[i].cur_dist, 0 );
		uprint( cbuf, length, 2 );
		uprint( "edges = ", 8, 0 );
		t_EDGE_LIST* tmp_entry = node_info[i].edges;
		while (tmp_entry != NULL) {
			length = int_print( cbuf, tmp_entry->ea, 0 );
			uprint( cbuf, length, 0 );
			if (tmp_entry->next  == NULL) {
				uprint( " ", 1, 2 );
				break;
			}
			else {
				uprint( ", ", 2, 0 );
				tmp_entry = tmp_entry->next;
			}
		}
		uprint( "cur_route = ", 12, 0 );
		t_NODE_LIST* tmp_node = node_info[i].cur_route;
		while (tmp_node != NULL) {
			length = int_print( cbuf, tmp_node->na, 0 );
			uprint( cbuf, length, 0 );
			if (tmp_node->next  == NULL) {
				uprint( " ", 1, 2 );
				break;
			}
			else {
				uprint( ", ", 2, 0 );
				tmp_node = tmp_node->next;
			}
		}
		uprint( " ", 1, 2 );
	}

	return 0;
}

*/
static void clearbss(void)
{
    unsigned int *p;
    extern unsigned int _bss_start[];
    extern unsigned int _bss_end[];

    for (p = _bss_start; p < _bss_end; p++) {
        *p = 0;
    }
}


void uprint( char* buf, int length, int ret ) {
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int* uart_out = (unsigned int*)0xc000fc00;
    unsigned int* uart_status = (unsigned int*)0xc000fc04;

	//unsigned int flg = 1;
	//while(flg == 1) {
		//flg = *uart_status;
	//}
	// *uart_out = 0x41;

	for (int i = 0; i < length + ret; i++) {
		unsigned int flg = 1;
		while(flg == 1) {
			flg = *uart_status;
		}
        *uart_out = ((i == length+1)&&(ret == 2)) ? 0x0a :
                    ((i == length)&&(ret == 1)) ? 0x20 :
                    ((i == length)&&(ret == 2)) ? 0x0d : buf[i];
		// *led = i;
	}
}

void pass() {
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int val;
    unsigned int timer,timer2;
    val = 0;
    while(1) {
		wait();
		val++;
		*led = val & 0x7777;
    }
}

void wait() {
    unsigned int timer,timer2;
    timer = 0;
	timer2 = 0;
    while(timer2 < LP2) {
        while(timer < LP) {
            timer++;
    	}
        timer2++;
	}
}

int int_print( char* cbuf, int value, int type ) {
	// type 0 : digit  1:hex
	unsigned char buf[32];
	int ofs = 0;
	int cntr = 0;
	if (value == 0) {
		cbuf[0] = 0x30;
		ofs = 1;
	}
	else if (type == 0) { // int
		if (value < 0) {
			cbuf[ofs++] = 0x2d;
			value = -value;
		}
		while(value > 0) {
			buf[cntr++] = (unsigned char)(value % 10);
			value = value / 10;
		}
		for(int i = cntr - 1; i >= 0; i--) {	
			cbuf[ofs++] = buf[i] + 0x30;
		}	
	}
	else { //unsinged int
		unsigned int uvalue = (unsigned int)value;
		while(uvalue > 0) {
			buf[cntr++] = (unsigned char)(uvalue % 10);
			uvalue = uvalue / 10;
		}
		for(int i = cntr - 1; i >= 0; i--) {	
			cbuf[ofs++] = buf[i] + 0x30;
		}	
	}
	return ofs;	
}

