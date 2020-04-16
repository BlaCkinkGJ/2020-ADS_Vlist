#include <iostream>
#include "vlist.h"

int main(void)
{
	struct vlist *vlist = vlist_alloc(NULL);

	struct sublist_node node;
	for (int i = 0; i < 100; i++) {
		node.size = sizeof(int);
		node.is_primitive = true;
		node.ivalue = i;

		vlist_add_sublist_node(vlist, &node);
	}

	std::cout << vlist_get_sublist_node(vlist, 0)->ivalue << std::endl;

	vlist_dealloc(vlist);
	return 0;
}
