/*
 * OF NUMA Parsing support.
 *
 * Copyright (C) 2015 - 2016 Cavium Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

<<<<<<< HEAD
=======
#define pr_fmt(fmt) "OF: NUMA: " fmt

>>>>>>> temp
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/nodemask.h>

#include <asm/numa.h>

/* define default numa node to 0 */
#define DEFAULT_NODE 0

/*
 * Even though we connect cpus to numa domains later in SMP
 * init, we need to know the node ids now for all cpus.
*/
static void __init of_numa_parse_cpu_nodes(void)
{
	u32 nid;
	int r;
	struct device_node *cpus;
	struct device_node *np = NULL;

	cpus = of_find_node_by_path("/cpus");
	if (!cpus)
		return;

	for_each_child_of_node(cpus, np) {
		/* Skip things that are not CPUs */
		if (of_node_cmp(np->type, "cpu") != 0)
			continue;

		r = of_property_read_u32(np, "numa-node-id", &nid);
		if (r)
			continue;

<<<<<<< HEAD
		pr_debug("NUMA: CPU on %u\n", nid);
		if (nid >= MAX_NUMNODES)
			pr_warn("NUMA: Node id %u exceeds maximum value\n",
				nid);
		else
			node_set(nid, numa_nodes_parsed);
	}
=======
		pr_debug("CPU on %u\n", nid);
		if (nid >= MAX_NUMNODES)
			pr_warn("Node id %u exceeds maximum value\n", nid);
		else
			node_set(nid, numa_nodes_parsed);
	}

	of_node_put(cpus);
>>>>>>> temp
}

static int __init of_numa_parse_memory_nodes(void)
{
	struct device_node *np = NULL;
	struct resource rsrc;
	u32 nid;
<<<<<<< HEAD
	int r = 0;

	for (;;) {
		np = of_find_node_by_type(np, "memory");
		if (!np)
			break;

=======
	int i, r;

	for_each_node_by_type(np, "memory") {
>>>>>>> temp
		r = of_property_read_u32(np, "numa-node-id", &nid);
		if (r == -EINVAL)
			/*
			 * property doesn't exist if -EINVAL, continue
			 * looking for more memory nodes with
			 * "numa-node-id" property
			 */
			continue;
<<<<<<< HEAD
		else if (r)
			/* some other error */
			break;

		r = of_address_to_resource(np, 0, &rsrc);
		if (r) {
			pr_err("NUMA: bad reg property in memory node\n");
			break;
		}

		pr_debug("NUMA:  base = %llx len = %llx, node = %u\n",
			 rsrc.start, rsrc.end - rsrc.start + 1, nid);

		r = numa_add_memblk(nid, rsrc.start,
				    rsrc.end - rsrc.start + 1);
		if (r)
			break;
	}
	of_node_put(np);

	return r;
=======

		if (nid >= MAX_NUMNODES) {
			pr_warn("Node id %u exceeds maximum value\n", nid);
			r = -EINVAL;
		}

		for (i = 0; !r && !of_address_to_resource(np, i, &rsrc); i++)
			r = numa_add_memblk(nid, rsrc.start, rsrc.end + 1);

		if (!i || r) {
			of_node_put(np);
			pr_err("bad property in memory node\n");
			return r ? : -EINVAL;
		}
	}

	return 0;
>>>>>>> temp
}

static int __init of_numa_parse_distance_map_v1(struct device_node *map)
{
	const __be32 *matrix;
	int entry_count;
	int i;

<<<<<<< HEAD
	pr_info("NUMA: parsing numa-distance-map-v1\n");

	matrix = of_get_property(map, "distance-matrix", NULL);
	if (!matrix) {
		pr_err("NUMA: No distance-matrix property in distance-map\n");
=======
	pr_info("parsing numa-distance-map-v1\n");

	matrix = of_get_property(map, "distance-matrix", NULL);
	if (!matrix) {
		pr_err("No distance-matrix property in distance-map\n");
>>>>>>> temp
		return -EINVAL;
	}

	entry_count = of_property_count_u32_elems(map, "distance-matrix");
	if (entry_count <= 0) {
<<<<<<< HEAD
		pr_err("NUMA: Invalid distance-matrix\n");
=======
		pr_err("Invalid distance-matrix\n");
>>>>>>> temp
		return -EINVAL;
	}

	for (i = 0; i + 2 < entry_count; i += 3) {
		u32 nodea, nodeb, distance;

		nodea = of_read_number(matrix, 1);
		matrix++;
		nodeb = of_read_number(matrix, 1);
		matrix++;
		distance = of_read_number(matrix, 1);
		matrix++;

		numa_set_distance(nodea, nodeb, distance);
<<<<<<< HEAD
		pr_debug("NUMA:  distance[node%d -> node%d] = %d\n",
=======
		pr_debug("distance[node%d -> node%d] = %d\n",
>>>>>>> temp
			 nodea, nodeb, distance);

		/* Set default distance of node B->A same as A->B */
		if (nodeb > nodea)
			numa_set_distance(nodeb, nodea, distance);
	}

	return 0;
}

static int __init of_numa_parse_distance_map(void)
{
	int ret = 0;
	struct device_node *np;

	np = of_find_compatible_node(NULL, NULL,
				     "numa-distance-map-v1");
	if (np)
		ret = of_numa_parse_distance_map_v1(np);

	of_node_put(np);
	return ret;
}

int of_node_to_nid(struct device_node *device)
{
	struct device_node *np;
	u32 nid;
	int r = -ENODATA;

	np = of_node_get(device);

	while (np) {
<<<<<<< HEAD
		struct device_node *parent;

=======
>>>>>>> temp
		r = of_property_read_u32(np, "numa-node-id", &nid);
		/*
		 * -EINVAL indicates the property was not found, and
		 *  we walk up the tree trying to find a parent with a
		 *  "numa-node-id".  Any other type of error indicates
		 *  a bad device tree and we give up.
		 */
		if (r != -EINVAL)
			break;

<<<<<<< HEAD
		parent = of_get_parent(np);
		of_node_put(np);
		np = parent;
	}
	if (np && r)
		pr_warn("NUMA: Invalid \"numa-node-id\" property in node %s\n",
			np->name);
	of_node_put(np);

	if (!r) {
		if (nid >= MAX_NUMNODES)
			pr_warn("NUMA: Node id %u exceeds maximum value\n",
				nid);
		else
			return nid;
	}
=======
		np = of_get_next_parent(np);
	}
	if (np && r)
		pr_warn("Invalid \"numa-node-id\" property in node %s\n",
			np->name);
	of_node_put(np);

	/*
	 * If numa=off passed on command line, or with a defective
	 * device tree, the nid may not be in the set of possible
	 * nodes.  Check for this case and return NUMA_NO_NODE.
	 */
	if (!r && nid < MAX_NUMNODES && node_possible(nid))
		return nid;
>>>>>>> temp

	return NUMA_NO_NODE;
}
EXPORT_SYMBOL(of_node_to_nid);

int __init of_numa_init(void)
{
	int r;

	of_numa_parse_cpu_nodes();
	r = of_numa_parse_memory_nodes();
	if (r)
		return r;
	return of_numa_parse_distance_map();
}
