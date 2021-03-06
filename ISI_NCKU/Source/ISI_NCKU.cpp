#include "../Headers/ISI_NCKU.h"

ISI_NCKU :: ISI_NCKU()
{
	UID = 0;
	k = -1;
	lb_index = 1;
	ub_index = 0;
	w = "waste"; m = "mix"; s = "split"; o = "output";
	o_count = 1;
}

//Function to print the tree in level order.
void ISI_NCKU :: printLevelOrder(node_ISI *R)
{
	if (!R)
		return;

	currentLevel.push(R);

	while (!currentLevel.empty())
	{
		node_ISI *currNode = currentLevel.front();
		currentLevel.pop();
		if (currNode)
		{
      			cout<<"[CF:"<<currNode->cf<<"->vc:"<<currNode->vertex_count<<"]\t";
      			//cout<<currNode->cf<<"\t";
			nextLevel.push(currNode->left);
			nextLevel.push(currNode->right);
		}

		if (currentLevel.empty())
		{
			cout <<"\n\n";
			swap(currentLevel, nextLevel);
		}
	}
}

//Function to store the nodes of the tree 'root' into vector 'nodes' in post-order traversal.
void ISI_NCKU :: postorder(node_ISI *T)
{
	if(T != NULL)
	{
		if(T->left)
			postorder(T->left);
		if(T->right)
			postorder(T->right);
		nodes.push_back(T);
	}
}

//Function to find the parent of a node in the tree
node_ISI* ISI_NCKU :: Find_parent(node_ISI *T, node_ISI *p, node_ISI *m)
{
	if(found)
		return p;

	if(T == m)
	{
		found = true;
		return p;
	}

	if(T->left != NULL)
	{
		node_ISI *tmp = Find_parent(T->left, T, m);
		if(tmp != NULL)
			return tmp;
	}

	if(T->right != NULL)
	{
		node_ISI *tmp = Find_parent(T->right, T, m);
		if(tmp != NULL)
			return tmp;
	}

	return NULL;
}

//LDT is based on Algorithm 1 from the paper. Please refer to the paper for details.
node_ISI* ISI_NCKU :: LDT(vector < int > l)
{
	node_ISI *v;
	if(l.size() == 1)
	{
		v = new node_ISI();
		v->id = UID++;
		v->cf = l[0];
		v->vertex_count = 0.0;
		v->left = NULL;
		v->right = NULL;
		nodes.push_back(v);	//Storing all the nodes intially created. Used to remove the unnecessary ones, if any.
	}
	else
	{
		int Cmid = l[ceil(l.size()/2)];

		vector < int > l_left;
		for(int i = 0; i<ceil(l.size()/2); i++)
			l_left.push_back(l[i]);

		vector < int > l_right;
		for(int i = ceil(l.size()/2) + 1; i<l.size(); i++)
			l_right.push_back(l[i]);

		node_ISI *v_left = LDT(l_left);
		node_ISI *v_right = LDT(l_right);

		v = new node_ISI();
		v->id = UID++;
		v->cf = Cmid;
		v->vertex_count = 0.0;
		v->left = v_left;
		v->right = v_right;
		nodes.push_back(v);	//Storing all the nodes intially created. Used to remove the unnecessary ones, if any.
	}
	return v;
}

void ISI_NCKU :: update_vertex_count(node_ISI *T, node_ISI *currNode, int diff, float add)
{
	if(T->cf == diff)
	{
		T->vertex_count += add;
		if(T == root)
			return;
		else
		{
			found = false;
			node_ISI *parent = Find_parent(root, NULL, T);
			parent->vertex_count += add/2;

			int diff1 = 2*T->cf - parent->cf;

			if((diff1 != LB) || (diff1 != UB))
				update_vertex_count(root, NULL, diff1, add/2);
		}
	}
	if(T->left != NULL)
		update_vertex_count(T->left, currNode, diff, add);
	if(T->right != NULL)
		update_vertex_count(T->right, currNode, diff, add);
}

vertices ISI_NCKU :: createVertices(node_ISI *T, Vertex *vs, Vertex *vm)
{
	vertices vertex;
	vertex.T = T;
	vertex.vmix = vm;
	vertex.vsplit.push_back(vs);
	vertex.vsplit.push_back(vs);
	tree_vertices.push_back(vertex);
	return vertex;
}

void ISI_NCKU :: createDag(DagGen *dag)
{
	Vertex *vm, *vs, *vw, *vo, *vl, *vr;
	vertices vertex, vertexl, vertexr;
	int diff;	//holds the difference between the 2*currNode's cf and its parent's CF so that we can know if the other node to be used is lower boundary node or upper boundary node or something else

	//Create mix and split vertices for the root
	vm = wara.createVertex(m_count, m, dag, MIX);
	vs = wara.createVertex(s_count, s, dag, SPLIT);
	//connect one lower and upper boundary node to the mix of the root
	dag->addEdge(boundary_vertices_wara[0][lb_index++], vm);
	dag->addEdge(boundary_vertices_wara[1][ub_index++], vm);
	//Connect mix to split vertex of root
	dag->addEdge(vm, vs);

	vertex = createVertices(root, vs, vm);

	if(root->left != NULL)
		currentLevel.push(root->left);
	if(root->right != NULL)
		currentLevel.push(root->right);

	while (!currentLevel.empty())
	{
		node_ISI *currNode = currentLevel.front();
		currentLevel.pop();

		//Create mix and split vertices for the currNode
		vm = wara.createVertex(m_count, m, dag, MIX);
		vs = wara.createVertex(s_count, s, dag, SPLIT);
		//Connect mix to split vertex
		dag->addEdge(vm, vs);

		vertex = createVertices(currNode, vs, vm);

		if(currNode->left == NULL && currNode->right == NULL)
		{
			if(currNode->cf <= max)
			{
				vo = wara.createVertex(o_count, o, dag, OUTPUT);
				//Connect the OUTPUT vertex to split.
				dag->addEdge(vs, vo);
			}
			else
			{
				vw = wara.createVertex(w_count, w, dag, WASTE);
				//Connect the Waste vertex to split.
				dag->addEdge(vs, vw);
			}
			currentLastLevel.push(vertex);
		}

		found = false;
		node_ISI *parent = Find_parent(root, NULL, currNode);
		parent->vertex_count += 0.5;

		diff = 2*currNode->cf - parent->cf;

		if(diff != LB && diff != UB)
			update_vertex_count(root, currNode, diff, 0.5);

		if(currNode->left != NULL)
			nextLevel.push(currNode->left);
		if(currNode->right != NULL)
			nextLevel.push(currNode->right);

		if (currentLevel.empty())
			swap(currentLevel, nextLevel);
	}

	//Generate the required number of nodes
	for(int i = 0; i<tree_vertices.size(); i++)
	{
		if(tree_vertices[i].vsplit.size() < 2*tree_vertices[i].T->vertex_count)
		{

		while(tree_vertices[i].vsplit.size() != 2*tree_vertices[i].T->vertex_count)
		{
			//Create mix and split vertices for the currNode
			vm = wara.createVertex(m_count, m, dag, MIX);
			vs = wara.createVertex(s_count, s, dag, SPLIT);
			//Connect mix to split vertex
			dag->addEdge(vm, vs);
			tree_vertices[i].vsplit.push_back(vs);
			tree_vertices[i].vsplit.push_back(vs);

			if(tree_vertices[i].T == root)
			{
				//connect one lower and upper boundary node to the mix of the root
				dag->addEdge(boundary_vertices_wara[0][lb_index++], vm);
				dag->addEdge(boundary_vertices_wara[1][ub_index++], vm);
			}
			else
			{
				found = false;
				node_ISI *parent = Find_parent(root, NULL, tree_vertices[i].T);
				for(int j = 0; j<tree_vertices.size(); j++)
					if(tree_vertices[j].T == parent)
					{
						vl = tree_vertices[j].vsplit[tree_vertices[j].vsplit.size()-1];
						dag->addEdge(vl, vm);
						tree_vertices[j].vsplit.pop_back();
						break;
					}
				diff = 2*tree_vertices[i].T->cf - parent->cf;

				if(diff == LB)	//connect one lower boundary node to the mix
					dag->addEdge(boundary_vertices_wara[0][lb_index++], vm);
				else if(diff == UB)	//connect one Upper boundary node to the mix
					dag->addEdge(boundary_vertices_wara[1][ub_index++], vm);
				else if(diff != LB && diff != UB)
				{
					for(int j = 0; j<tree_vertices.size(); j++)
						if(tree_vertices[j].T->cf == diff)
						{
							vr = tree_vertices[j].vsplit[tree_vertices[j].vsplit.size()-1];
							dag->addEdge(vr, vm);
							tree_vertices[j].vsplit.pop_back();
							break;
						}
				}
			}
		}

		}
	}

	//Connect the edges of all nodes except the root.
	for(int i = 1; i<tree_vertices.size(); i++)
	{
		found = false;
		node_ISI *parent = Find_parent(root, NULL, tree_vertices[i].T);
		for(int j = 0; j<tree_vertices.size(); j++)
			if(tree_vertices[j].T == parent)
			{
				vl = tree_vertices[j].vsplit[tree_vertices[j].vsplit.size()-1];
				dag->addEdge(vl, tree_vertices[i].vmix);
				tree_vertices[j].vsplit.pop_back();
				break;
			}
		diff = 2*tree_vertices[i].T->cf - parent->cf;

		if(diff == LB)	//connect one lower boundary node to the mix
			dag->addEdge(boundary_vertices_wara[0][lb_index++], tree_vertices[i].vmix);
		else if(diff == UB)	//connect one Upper boundary node to the mix
			dag->addEdge(boundary_vertices_wara[1][ub_index++], tree_vertices[i].vmix);
		else if(diff != LB && diff != UB)
		{
			for(int j = 0; j<tree_vertices.size(); j++)
				if(tree_vertices[j].T->cf == diff)
				{
					vr = tree_vertices[j].vsplit[tree_vertices[j].vsplit.size()-1];
					dag->addEdge(vr, tree_vertices[i].vmix);
					tree_vertices[j].vsplit.pop_back();
					break;
				}
		}
	}

	while(!currentLastLevel.empty())
	{
		vertexl = currentLastLevel.front();
		vl = vertexl.vsplit[0];
		currentLastLevel.pop();

		if(currentLastLevel.empty())
		{
			vw = wara.createVertex(w_count, w, dag, WASTE);
			//Connect the OUTPUT vertex to split.
			dag->addEdge(vl, vw);
			break;
		}
		vertexr = currentLastLevel.front();
		vr = vertexr.vsplit[0];
		currentLastLevel.pop();

		//Create mix and split vertices for the currNode
		vm = wara.createVertex(m_count, m, dag, MIX);
		vs = wara.createVertex(s_count, s, dag, SPLIT);
		//Connect mix to split vertex
		dag->addEdge(vm, vs);
		dag->addEdge(vl, vm);
		dag->addEdge(vr, vm);
		if((vertexl.T->cf + vertexr.T->cf)/2 <= max)
		{
			vo = wara.createVertex(o_count, o, dag, OUTPUT);
			//Connect the OUTPUT vertex to split.
			dag->addEdge(vs, vo);
		}
		else
		{
			vw = wara.createVertex(w_count, w, dag, WASTE);
			//Connect the OUTPUT vertex to split.
			dag->addEdge(vs, vw);
		}

		for(int i = 0; i<tree_vertices.size(); i++)
		{
			if(tree_vertices[i].T->cf == (vertexl.T->cf + vertexr.T->cf)/2)
			{
				while(!tree_vertices[i].vsplit.empty())
					tree_vertices[i].vsplit.pop_back();
				tree_vertices[i].vsplit.push_back(vs);
				nextLastLevel.push(tree_vertices[i]);
				break;
			}
		}

		if (currentLastLevel.empty())
			swap(currentLastLevel, nextLastLevel);
	}
}
void ISI_NCKU :: RUN_NCKU(std::vector<std::string> parameters, DagGen *dag)
{
	ISI_NCKU ncku;
	return ncku.RUN_NCKU_Internal(parameters,dag);
}
void ISI_NCKU :: RUN_NCKU_Internal(std::vector<std::string> parameters, DagGen *dag)
{
	if(dag ==NULL)
		dag = new DagGen();
	a = atoi(parameters[0].c_str());
	d = atoi(parameters[1].c_str());
	n = atoi(parameters[2].c_str());
	count = atoi(parameters[3].c_str());

	//Using the formula, count = 2^(k+1) + 1, we find 'k' from count i.e. k = ((count-1)/2)^(1/2)
	int tmp = (count - 1)/2;
	while(tmp != 0)
	{
		k++;
		tmp /= 2;
	}

	//If the count of CF is greater than 2^(k+1) + 1, we will have to increament 'k' by 1. E.g., if 'k = 2' and 'count = 11', 2^(k+1) + 1 = 9. Since 11>9, we choose 'k = 3' so that 2^(k+1) + 1 = 16. So, we will get the required CF's from 'root' and delete the unwanted once.
	if(count > (pow(2,k+1)+1))
		k++;

	//Obviously the lower boundary CF is 'a' itself and the Upper boundary CF is 'a+(d*pow(2,k+1))'.
	LB = a;
	UB = a+(d*pow(2,k+1));
	max = a+(d*(count-1));

	for(int i = 0; i<=pow(2,k); i++)
		av.push_back(LB);
	if(count == (pow(2,k+1)+1))
		for(int i = 0; i<=pow(2,k); i++)
			av.push_back(UB);
	else
		for(int i = 0; i<pow(2,k); i++)
			av.push_back(UB);

	av.push_back(pow(2,n));

	//Using logic from Algorithm 2
	//We push all the CF's except LB & UB into the target set L.
	for(int i=1; i<pow(2,k+1); i++)
		L.push_back(a+(d*i));

	//We call the LDT function to build the Linear Dilution Tree
	root = LDT(L);

	while(!nodes.empty())	//empty the nodes vector to store the remaining nodes in post-order traversal
		nodes.pop_back();

	//Storing the tree nodes in the vector nodes in the post-order traversal of the tree.
	postorder(root);
	//Algorithm 2 ends here

	boundary_vertices_wara = wara.WARA_MAIN(dag, av);

	w_count = wara.w_count;
	m_count = wara.m_count;
	s_count = wara.s_count;

	Vertex *vo_lb = wara.createVertex(o_count, o, dag, OUTPUT);
	//Connect one of the Split vertex for lower bound to the OUTPUT vertex.
	dag->addEdge(boundary_vertices_wara[0][0], vo_lb);

	if(count == (pow(2,k+1)+1))
	{
		Vertex *vo_ub = wara.createVertex(o_count, o, dag, OUTPUT);
		//Connect one of the Split vertex for upper bound to the OUTPUT vertex.
		dag->addEdge(boundary_vertices_wara[1][ub_index++], vo_ub);
	}

	createDag(dag);

	//printLevelOrder(root);

    //	dag->generateJSON("Example.json");
	//dag->generateDropletDag("Dropletdag->cpp");
	//dag->generateDotyGraph("Example.dot");

	return;
}
