#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct cell_s cell_t;

struct block_s {
	int type;
	cell_t *cell;
};
typedef struct block_s block_t;

typedef struct link_s link_t;

struct cell_s {
	block_t *block;
	unsigned long value;
	int visited;
	unsigned long links_n;
	link_t *links;
	cell_t *from;
	int direction;
};

struct link_s {
	cell_t *cell;
	int direction;
};

int set_block(block_t *, int);
int set_cell(cell_t *, unsigned long, unsigned long, block_t *);
int yx_links(block_t *, cell_t *, int, int);
int yx_link(block_t *, cell_t *, int);
int add_link(cell_t *, cell_t *, int);
void set_link(link_t *, cell_t *, int);
void bfs_path(cell_t *, cell_t *);
void bfs_test_link(cell_t *, link_t *);
int sort_links(const void *, const void *);
void dfs_path(unsigned long, cell_t *);
void dfs_test_link(cell_t *, link_t *, unsigned long);
void free_cells(void);

unsigned long height, width, blocks_n, cells_n, baloneys_n, queue_size, path_len_min, baloneys_found;
block_t *blocks;
cell_t *cells, *cell_start, **baloneys, **queue;

int main(void) {
unsigned long b, c, i, j;
	scanf("%lu", &height);
	if (!height) {
		fprintf(stderr, "Height must be greater than 0\n");
		return EXIT_FAILURE;
	}
	scanf("%lu", &width);
	if (!width) {
		fprintf(stderr, "Width must be greater than 0\n");
		return EXIT_FAILURE;
	}
	fgetc(stdin);
	blocks_n = width*height;
	blocks = malloc(sizeof(block_t)*blocks_n);
	if (!blocks) {
		fprintf(stderr, "Could not allocate blocks\n");
		return EXIT_FAILURE;
	}
	cells_n = 0;
	b = 0;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++, b++) {
			if (set_block(blocks+b, fgetc(stdin))) {
				free(blocks);
				return EXIT_FAILURE;
			}
		}
		fgetc(stdin);
	}
	if (!cells_n) {
		fprintf(stderr, "Number of cells must be greater than 0\n");
		free(blocks);
		return EXIT_FAILURE;
	}
	cells = malloc(sizeof(cell_t)*cells_n);
	if (!cells) {
		fprintf(stderr, "Could not allocate cells\n");
		free(blocks);
		return EXIT_FAILURE;
	}
	cell_start = NULL;
	baloneys_n = 0;
	b = 0;
	c = 0;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++, b++) {
			if (blocks[b].type != 'O') {
				if (set_cell(cells+c, i, j, blocks+b)) {
					free_cells();
					free(blocks);
					return EXIT_FAILURE;
				}
				c++;
			}
		}
	}
	if (!cell_start) {
		fprintf(stderr, "Start not set\n");
		free_cells();
		free(blocks);
		return EXIT_FAILURE;
	}
	if (!baloneys_n) {
		fprintf(stderr, "Number of baloneys must be greater than 0\n");
		free_cells();
		free(blocks);
		return EXIT_FAILURE;
	}
	queue = malloc(sizeof(cell_t *)*cells_n);
	if (!queue) {
		fprintf(stderr, "Could not allocate queue\n");
		free_cells();
		free(blocks);
		return EXIT_FAILURE;
	}
	for (i = 0; i < baloneys_n; i++) {
		bfs_path(cell_start, baloneys[i]);
	}
	for (i = 0; i < baloneys_n-1; i++) {
		for (j = i+1; j < baloneys_n; j++) {
			bfs_path(baloneys[i], baloneys[j]);
		}
	}
	free(queue);
	for (i = 0; i < cells_n; i++) {
		qsort(cells[i].links, cells[i].links_n, sizeof(link_t), sort_links);
	}
	cell_start->visited = 1;
	cell_start->from = NULL;
	path_len_min = cells_n;
	baloneys_found = 0;
	dfs_path(1UL, cell_start);
	free_cells();
	free(blocks);
	return EXIT_SUCCESS;
}

int set_block(block_t *block, int type) {
	switch (type) {
	case 'S':
	case '*':
	case '.':
		cells_n++;
		break;
	case 'O':
		block->cell = NULL;
		break;
	default:
		fprintf(stderr, "Invalid type\n");
		return 1;
	}
	block->type = type;
	return 0;
}

int set_cell(cell_t *cell, unsigned long y, unsigned long x, block_t *block) {
cell_t **baloneys_tmp;
	block->cell = cell;
	cell->block = block;
	if (block->type == 'S') {
		if (cell_start) {
			fprintf(stderr, "Start already set\n");
			return 1;
		}
		else {
			cell_start = cell;
		}
	}
	else if (block->type == '*') {
		if (baloneys_n) {
			baloneys_tmp = realloc(baloneys, sizeof(cell_t *)*(baloneys_n+1));
			if (baloneys_tmp) {
				baloneys = baloneys_tmp;
			}
			else {
				fprintf(stderr, "Could not reallocate baloneys\n");
				return 1;
			}
		}
		else {
			baloneys = malloc(sizeof(cell_t *));
			if (!baloneys) {
				fprintf(stderr, "Could not allocate baloneys\n");
				return 1;
			}
		}
		baloneys[baloneys_n++] = cell;
	}
	cell->value = 0;
	cell->visited = 0;
	cell->links_n = 0;
	if (y && yx_links(block-width, cell, 'd', 'u')) {
		return 1;
	}
	if (x && yx_links(block-1, cell, 'r', 'l')) {
		return 1;
	}
	return 0;
}

int yx_links(block_t *remote, cell_t *cell, int direction1, int direction2) {
	if (yx_link(remote, cell, direction1)) {
		return 1;
	}
	if (remote->cell && yx_link(cell->block, remote->cell, direction2)) {
		return 1;
	}
	return 0;
}

int yx_link(block_t *block1, cell_t *cell2, int direction) {
	return block1->type != 'O' ? add_link(block1->cell, cell2, direction):0;
}

int add_link(cell_t *cell1, cell_t *cell2, int direction) {
link_t *links_tmp;
	if (cell1->links_n) {
		links_tmp = realloc(cell1->links, sizeof(link_t)*(cell1->links_n+1));
		if (links_tmp) {
			cell1->links = links_tmp;
		}
		else {
			fprintf(stderr, "Could not reallocate links\n");
			free(cell1->links);
			return 1;
		}
	}
	else {
		cell1->links = malloc(sizeof(link_t));
		if (!cell1->links) {
			fprintf(stderr, "Could not allocate links\n");
			return 1;
		}
	}
	set_link(cell1->links+cell1->links_n, cell2, direction);
	cell1->links_n++;
	return 0;
}

void set_link(link_t *link, cell_t *cell, int direction) {
	link->cell = cell;
	link->direction = direction;
}

void bfs_path(cell_t *from, cell_t *to) {
unsigned long i, j, value;
cell_t *cell;
	from->visited = 1;
	from->from = NULL;
	queue[0] = from;
	queue_size = 1;
	for (i = 0; i < queue_size && queue[i] != to; i++) {
		for (j = 0; j < queue[i]->links_n; j++) {
			bfs_test_link(queue[i], queue[i]->links+j);
		}
	}
	if (i < queue_size) {
		value = 1;
		for (cell = queue[i]; cell->from; cell = cell->from) {
			if (cell->block->type == '*') {
				value++;
			}
		}
		for (cell = queue[i]; cell; cell = cell->from) {
			if (cell->value < value) {
				cell->value = value;
			}
		}
	}
	for (i = 0; i < queue_size; i++) {
		queue[i]->visited = 0;
	}
}

void bfs_test_link(cell_t *from, link_t *to) {
	if (!to->cell->visited) {
		to->cell->visited = 1;
		to->cell->from = from;
		queue[queue_size++] = to->cell;
	}
}

int sort_links(const void *a, const void *b) {
const link_t *link_a = (const link_t *)a, *link_b = (const link_t *)b;
	if (link_a->cell->value < link_b->cell->value) {
		return 1;
	}
	else if (link_a->cell->value > link_b->cell->value) {
		return -1;
	}
	else {
		return 0;
	}
}

void dfs_path(unsigned long path_len, cell_t *cell) {
unsigned long i;
	if (path_len <= path_len_min-baloneys_n+baloneys_found) {
		for (i = 0; i < cell->links_n; i++) {
			dfs_test_link(cell, cell->links+i, path_len);
		}
	}
}

void dfs_test_link(cell_t *from, link_t *to, unsigned long path_len) {
unsigned long b, i, j;
cell_t *cell;
	if (!to->cell->visited) {
		from->direction = to->direction;
		to->cell->visited = 1;
		to->cell->from = from;
		if (to->cell->block->type == '*') {
			baloneys_found++;
			if (baloneys_found == baloneys_n) {
				for (cell = to->cell->from; cell; cell = cell->from) {
					cell->block->type = cell->block->type == '.' ? cell->direction:toupper(cell->direction);
				}
				printf("%lu\n", path_len);
				b = 0;
				for (i = 0; i < height; i++) {
					for (j = 0; j < width; j++, b++) {
						putchar(blocks[b].type);
					}
					puts("");
				}
				path_len_min = path_len;
			}
		}
		if (baloneys_found < baloneys_n) {
			dfs_path(path_len+1, to->cell);
		}
		if (to->cell->block->type == '*') {
			if (baloneys_found == baloneys_n) {
				for (cell = to->cell->from; cell->from; cell = cell->from) {
					cell->block->type = islower(cell->block->type) ? '.':'*';
				}
				cell->block->type = 'S';
			}
			baloneys_found--;
		}
		to->cell->visited = 0;
	}
}

void free_cells(void) {
unsigned long i;
	if (baloneys_n) {
		free(baloneys);
	}
	for (i = 0; i < cells_n; i++) {
		if (cells[i].links_n) {
			free(cells[i].links);
		}
	}
	free(cells);
}
