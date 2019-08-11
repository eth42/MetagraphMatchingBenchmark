#include "CherryTree.h"

namespace maxmatching {
	template <class Label>
	CherryTree<Label>::CherryTree(Vertex<Label>* root)
		: consistent(true)
		, root(root)
		, size(0) {
		root->setContainingTree(this);
		Statistics::incrementTreeCreated();
	}


	template <class Label>
	CherryTree<Label>::~CherryTree() {
		DEBUG("Deleting tree " << (this->root == nullptr ? 0 : this->root->id) << "\n");
		this->printTree();
		/* Prevent any prints during deletion */
		consistent = false;
		/* Remove all vertices from the tree, so the vertices can't reference the tree anymore. */
		auto containedVertices = this->getVertices();
		while (!containedVertices->isEmpty()) {
			Vertex<Label>* v = containedVertices->pop();
			v->setContainingTree(nullptr);
		}
		containedVertices->deleteStructure();
		Statistics::incrementTreeDeleted();
	}

	/* Rotates the tree according to the pseudocode. */
	template <class Label>
	void CherryTree<Label>::rotate(Vertex<Label>* newRoot) {
		DEBUG("Rotating tree " << this->root->id << " to " << newRoot->id << "\n");
		bool consistencyUpdate = this->consistent;
		consistent = false;
		List<Vertex<Label>> * l = new List<Vertex<Label>>();
		CherryBlossom<Label> * blossom;
		Vertex<Label> * u = newRoot;
		while (u != this->root) {
			l->append(u);
			blossom = u->getContainingBlossom();
			while (blossom != nullptr) {
				u = blossom->getReceptacle();
				blossom = u->getContainingBlossom();
			}
			Vertex<Label>* s = u->getMatchingPartner();
			/* Only the root has no matching partner, thus this equals the break */
			if (s != nullptr) {
				l->append(s);
				u = s->getEvenParent();
			}
		}
		/* Sanity check, this should ALWAYS be the case */
		if (l->getSize() % 2 == 0) {
			l->append(this->root);
		}
		List<CherryBlossom<Label>> rotateBlossoms;
		List<Vertex<Label>> newReceptacles;
		u = l->pop();
		blossom = u->getContainingBlossom();
		while (blossom != nullptr) {
			rotateBlossoms.push(blossom);
			newReceptacles.push(u);
			u = blossom->getReceptacle();
			blossom = u->getContainingBlossom();
		}
		while (!rotateBlossoms.isEmpty()) {
			blossom = rotateBlossoms.pop();
			blossom->rotate(newReceptacles.pop());
		}
		while (!l->isEmpty()) {
			Vertex<Label>* s = l->pop();
			Vertex<Label>* t = l->pop();
			s->setEvenParent(u);
			blossom = t->getContainingBlossom();
			u = t;
			while (blossom != nullptr) {
				rotateBlossoms.push(blossom);
				newReceptacles.push(u);
				u = blossom->getReceptacle();
				blossom = u->getContainingBlossom();
			}
			while (!rotateBlossoms.isEmpty()) {
				blossom = rotateBlossoms.pop();
				blossom->rotate(newReceptacles.pop());
			}
			s->setMatchingPartner(t);
			t->setMatchingPartner(s);
		}
		l->deleteStructure();
		newRoot->setMatchingPartner(nullptr);
		this->root = newRoot;
		/* Since trees are immediately destroyed after rotating, we don't need
		 * to update the levels. Consistency of the tree is not an issue.
		 * If anyone changes the code to use the trees after any rotation, this has to
		 * be commented back in! */
		 //this->updateLevel();
		consistent = consistencyUpdate;
		this->printTree();
	}

	/* This is to be called, if the receptacle has just been added and the other vertices
	 * of the corolla are in no tree yet. It claims the corolla for this tree and manages
	 * the children of the new nodes. */
	template <class Label>
	void CherryTree<Label>::adoptBlossom(CherryBlossom<Label>* blossom) {
		DEBUG("Adopting blossom " << blossom->getReceptacle()->id << "\n");
		bool consistencyUpdate = this->consistent;
		this->consistent = false;
		auto fun = [this, blossom](Vertex<Label> * v)->void {
			DEBUG("Visiting " << v->id << "\n");
			v->setContainingTree(this);
			v->enqueue();
			this->size += 1;
			/* Adopt further blossoms */
			for (auto el = v->bearingBlossoms.getFirstElement(); el != nullptr; el = el->nxtElem) {
				this->adoptBlossom(el->value->getReference());
			}
			/* Add children to the tree */
			for (auto elem = v->oddChildren.getFirstElement(); elem != nullptr; elem = elem->nxtElem) {
				Vertex<Label>* child = elem->value;
				if (child->getContainingBlossom() != blossom) {
					this->add(v, child);
				}
			}
		};
		blossom->updateLevel();
		blossom->foreachVertex(fun);
		this->consistent = consistencyUpdate;
	}

	/* Adds a vertex into the tree with the given parent node as even parent.
	 * Also uses as much of potential old tree structure as possible
	 * given the information stored in the vertices. */
	template <class Label>
	void CherryTree<Label>::add(Vertex<Label> * parent, Vertex<Label> * newVertex) {
		/* Sanity check */
		if (newVertex->getContainingTree() != nullptr) {
			return;
		}
		bool consistencyUpdate = this->consistent;
		this->consistent = false;
		/* Update references for containment in tree */
		Vertex<Label>* newPartner = newVertex->getMatchingPartner();
		DEBUG("Adding vertices "
			<< parent->id << " -> "
			<< newVertex->id << " -> "
			<< newPartner->id << "\n");
		newVertex->setEvenParent(parent);
		newVertex->setContainingTree(this);
		newPartner->setContainingTree(this);
		newPartner->enqueue();
		this->size += 2;
		/* Remove all odd children of the new node, since odd and non-even nodes can't have odd children */
		while (!newVertex->oddChildren.isEmpty()) {
			ListElement<Vertex<Label>>* elem = newVertex->oddChildren.popElem();
			elem->value->setEvenParent(nullptr);
		}
		/* If this gets added, the old blossom should be dead for consistency reasons */
		if (newVertex->getContainingBlossom() != nullptr) {
			delete(newVertex->getContainingBlossom());
		}
		/* Delete potentially borne blossoms by now odd and non-even node */
		while (!newVertex->bearingBlossoms.isEmpty()) {
			delete(newVertex->bearingBlossoms.popElem()->value);
		}
		/* Adopt blossoms borne by the matching partner, which is even in this tree */
		for (auto el = newPartner->bearingBlossoms.getFirstElement(); el != nullptr; el = el->nxtElem) {
			this->adoptBlossom(el->value->getReference());
		}
		/* Call recursive, if this Vertex<Label> was hosting an old twig */
		DEBUG("Node " << newPartner->id << " has " << newPartner->oddChildren.getSize() << " odd children\n");
		for (auto elem = newPartner->oddChildren.getFirstElement(); elem != nullptr; elem = elem->nxtElem) {
			Vertex<Label>* child = elem->value;
			DEBUG("Node " << newPartner->id << " considers child " << child->id << "\n");
			CherryBlossom<Label> * childBlossom = child->getContainingBlossom();
			if (childBlossom == nullptr || childBlossom->getReceptacle() != newPartner) {
				/* Otherwise this should've been handled by adopting the blossom */
				this->add(newPartner, child);
			} else {
				DEBUG("Node " << newPartner->id << " ignores child " << child->id << "\n");
			}
		}
		/* Safe upwards in the trees */
		Vertex<Label>* oldParent = newPartner->getEvenParent();
		if (oldParent != nullptr) {
			newPartner->setEvenParent(nullptr);
			if (oldParent->getContainingTree() != newPartner->getContainingTree()) {
				this->add(newPartner, oldParent);
			}
			/* Creating blossoms between two even nodes is dangerous
			 * while the tree is inconsistent!
			 * If anyone ever intends to do so, beware of hard bugs to catch. */
		}
		this->consistent = consistencyUpdate;
		this->printTree();
	}

	/* Removes a vertex from the tree. Also removes all vertices hanging below
	 * this vertex. However this does not clear the evenParent references and
	 * blossoms, so the old structure can be reintegrated into different trees. */
	template <class Label>
	void CherryTree<Label>::remove(Vertex<Label> * v) {
		bool consistencyUpdate = this->consistent;
		this->consistent = false;
		if (v->getContainingTree() == this) {
			v->setContainingTree(nullptr);
			if (v == this->root) {
				this->root = nullptr;
			}
			Vertex<Label>* m = v->getMatchingPartner();
			this->size--;
			if (m != nullptr) {
				this->remove(m);
			}
			for (auto el = v->oddChildren.getFirstElement(); el != nullptr; el = el->nxtElem) {
				this->remove(el->value);
			}
		}
		this->consistent = consistencyUpdate;
	}

	/* Checks if a blossom creation is necessary and if so performs it
	 * according to the pseudocode. */
	template <class Label>
	void CherryTree<Label>::makeBlossom(Vertex<Label> * u, Vertex<Label> * w) {
		DEBUG("Checking blossom creation between " << u->id << " and " << w->id << "\n");
		CherryBlossom<Label> * uBlossom = u->getContainingBlossom();
		CherryBlossom<Label> * wBlossom = w->getContainingBlossom();
		CherryBlossom<Label> * blossom;
		/* Return condition: Both vertices are in the same blossom.
		 * Either they are in the same corolla
		 * or one of the vertices is the receptacle for the blossom
		 * of which the other is in the corolla */
		if (uBlossom != nullptr) {
			if (uBlossom == wBlossom || uBlossom->getReceptacle() == w) {
				return;
			}
		} else if (wBlossom != nullptr && wBlossom->getReceptacle() == u) {
			return;
		}
		this->printTree();
		bool consistencyUpdate = this->consistent;
		this->consistent = false;
		/* Using the level of the vertices, the search for a common even
		 * ancestor is performed by stepwise creation of the paths from the
		 * vertex with the higher level. In the worst case, this results in
		 * the same result as the pseudocode, creating paths to the root of
		 * the tree, but on average, this ends up at the demanded vertex. */
		Vertex<Label>* s = u;
		Vertex<Label>* t = w;
		CherryBlossom<Label>* possibleReuseBlossom = nullptr;
		std::vector<Vertex<Label>*> ss;
		ss.push_back(s);
		std::vector<Vertex<Label>*> ts;
		ts.push_back(t);
		while (s != t) {
			DEBUG(
				"s: " << (s == nullptr ? 0 : s->id) <<
				" (" << (s == nullptr ? 0 : s->getLevel()) << ")" <<
				" in " << (s == nullptr || s->getContainingBlossom() == nullptr || s->getContainingBlossom()->getReceptacle() == nullptr ? 0 : s->getContainingBlossom()->getReceptacle()->id) <<
				" @ " << (s == nullptr || s->getContainingBlossom() == nullptr ? 0 : s->getContainingBlossom()) <<
				" (" << (s == nullptr || s->getContainingBlossom() == nullptr ? 0 : s->getContainingBlossom()->getLevel()) << ")" <<
				" t: " << (t == nullptr ? 0 : t->id) <<
				" (" << (t == nullptr ? 0 : t->getLevel()) << ")" <<
				" in " << (t == nullptr || t->getContainingBlossom() == nullptr || t->getContainingBlossom()->getReceptacle() == nullptr ? 0 : t->getContainingBlossom()->getReceptacle()->id) <<
				" @ " << (t == nullptr || t->getContainingBlossom() == nullptr ? 0 : t->getContainingBlossom()) <<
				" (" << (t == nullptr || t->getContainingBlossom() == nullptr ? 0 : t->getContainingBlossom()->getLevel()) << ")" << "\n");
			if (s->getLevel() < t->getLevel()) {
				/* Copied code skips a variable switch */
				blossom = t->getContainingBlossom();
				if (blossom != nullptr) {
					t = blossom->getReceptacle();
					ts.push_back(t);
				} else {
					t = t->getMatchingPartner()->getEvenParent();
					ts.push_back(t);
				}
			} else {
				blossom = s->getContainingBlossom();
				if (blossom != nullptr) {
					s = blossom->getReceptacle();
					ss.push_back(s);
				} else {
					s = s->getMatchingPartner()->getEvenParent();
					ss.push_back(s);
				}
			}
		}
		/* Backtrack in case the level strategy didn't work.
		 * This happens in very rare cases, when the receptacle of a blossom
		 * is added to another blossom further up the tree. It is a VERY rare
		 * case and led to some serious debugging effort, since even finding
		 * an instance in which this happens is searching a needle in the hay. */
		while (ss.size() > 0 && ts.size() > 0 && ss.back() == ts.back()) {
			s = ss.back();
			ss.pop_back();
			ts.pop_back();
		}
		/* Reuse a blossom, if it is on the paths and the receptacle is the common ancestor. */
		if (ss.size() > 0) {
			Vertex<Label>* sb = ss.back();
			CherryBlossom<Label>* blossom = sb->getContainingBlossom();
			if (blossom != nullptr) {
				possibleReuseBlossom = blossom;
			}
		}
		if (ts.size() > 0) {
			Vertex<Label>* tb = ts.back();
			CherryBlossom<Label>* blossom = tb->getContainingBlossom();
			if (blossom != nullptr) {
				possibleReuseBlossom = blossom;
			}
		}
		ss.clear();
		ts.clear();
		Vertex<Label>* r = s;
		CherryBlossom<Label>* newBlossom;
		/* Try to reuse an existing blossom, through which one of both pathes must have gone */
		if (possibleReuseBlossom != nullptr && possibleReuseBlossom->getReceptacle() == r) {
			DEBUG("Reusing blossom with receptacle" << r->id << "\n");
			newBlossom = possibleReuseBlossom;
		} else {
			DEBUG("New blossom with receptacle " << r->id << "\n");
			newBlossom = new CherryBlossom<Label>(r);
		}
		/* Remember NOT to change parents if we are reusing this blossom */
		bool uFreeze = (uBlossom == newBlossom) || (u == r);
		bool wFreeze = (wBlossom == newBlossom) || (w == r);
		auto newBlossomRef = newBlossom->getReference();
		/* This is all pseudocode from here on */
		for (int i = 0; i < 2; i++) {
			s = (i == 0 ? u : w);
			while (s != r) {
				blossom = s->getContainingBlossom();
				if (blossom != nullptr) {
					if (blossom != newBlossomRef) {
						Vertex<Label>* r2 = blossom->getReceptacle();
						if (r != r2) {
							while (s != r2) {
								Vertex<Label>* t = s->getMatchingPartner()->getEvenParent();
								/* The recepticle might actually be part of the new blossom in which case it is not to be modified */
								if (t->getContainingBlossom() != newBlossomRef) {
									t->setEvenParent(s->getMatchingPartner());
								}
								s = t;
							}
						} else {
							s = r;
						}
						newBlossom->merge(blossom);
						newBlossomRef = newBlossom->getReference();
					} else {
						s = r;
					}
				} else {
					newBlossom->add(s);
					newBlossom->add(s->getMatchingPartner());
					Vertex<Label>* t = s->getMatchingPartner()->getEvenParent();
					if (t != r && t->getContainingBlossom() != newBlossomRef) {
						t->setEvenParent(s->getMatchingPartner());
					}
					s = t;
				}
			}
		}
		if (!uFreeze) {
			u->setEvenParent(w);
		}
		if (!wFreeze) {
			w->setEvenParent(u);
		}
		this->consistent = consistencyUpdate;
		this->printTree();
	}

	/* Updates the level of all vertices */
	template <class Label>
	void CherryTree<Label>::updateLevel() {
		this->updateLevelBelow(this->root);
	}

	/* Updates the level of all vertices below the given vertex */
	template <class Label>
	void CherryTree<Label>::updateLevelBelow(Vertex<Label> * v) {
		List<Vertex<Label>>* remainingVertices = new List<Vertex<Label>>();
		remainingVertices->push(v);
		while (!remainingVertices->isEmpty()) {
			Vertex<Label>* w = remainingVertices->pop();
			w->updateLevel();
			for (auto childEl = w->oddChildren.getFirstElement(); childEl != nullptr; childEl = childEl->nxtElem) {
				Vertex<Label>* m = childEl->value->getMatchingPartner();
				remainingVertices->append(m);
			}
		}
		remainingVertices->deleteStructure();
	}

	template <class Label>
	List<Vertex<Label>>* CherryTree<Label>::getVertices() {
		List<Vertex<Label>>* ret = new List<Vertex<Label>>();
		if (this->root == nullptr) {
			return ret;
		}
		ret->append(this->root);
		for (auto el = ret->getFirstElement(); el != nullptr; el = el->nxtElem) {
			Vertex<Label>* v = el->value;
			for (auto el2 = v->oddChildren.getFirstElement(); el2 != nullptr; el2 = el2->nxtElem) {
				Vertex<Label>* w = el2->value;
				/* The only way we're not revisiting w is if it's not even */
				if (!w->isEven()) {
					ret->append(w);
				}
				ret->append(w->getMatchingPartner());
			}
		}
		return ret;
	}


	template <class Label>
	int CherryTree<Label>::printCtr = 0;

	template <class Label>
	void CherryTree<Label>::logTree() {
		std::stringstream stream;
		recursivePrintNode(root, "", stream);
		std::string buffer = stream.str();
		DEBUG(buffer);
	}

	/* Prints the tree for debugging purposes. This is disabled if not using debugging. */
	template <class Label>
	void CherryTree<Label>::printTree() {
#ifndef DEBUG_F
		return;
#endif
		if (!this->consistent) {
			return;
		}
		DEBUG("@Tree(" << (this->root == nullptr ? 0 : this->root->id) << ") ");
		std::stringstream gmlStream, dotStream;
		gmlStream << "graph [\n";
		dotStream << "digraph G {\n";
		auto vs = this->getVertices();
		for (auto it = vs->getFirstElement(); it != nullptr; it = it->nxtElem) {
			Vertex<Label>* v = it->value;
			gmlStream << "\tnode [\n\t\tid " << v->id << "\n\t\tlabel \"" << v->id << "\"\n\t]\n";
		}
		for (auto it = vs->getFirstElement(); it != nullptr; it = it->nxtElem) {
			Vertex<Label>* v = it->value;
			Vertex<Label>* m = v->getMatchingPartner();
			Vertex<Label>* p = v->getEvenParent();
			if (m != nullptr) {
				gmlStream << "\tedge [\n\t\tsource " << v->id << "\n\t\ttarget " << m->id << "\n\t\tlabel \"m\"\n\t]\n";
				dotStream << "\t" << v->id << " -> " << m->id << "[color=red,penwidth=2.0];\n";
			}
			if (p != nullptr) {
				gmlStream << "\tedge [\n\t\tsource " << p->id << "\n\t\ttarget " << v->id << "\n\t]\n";
				dotStream << "\t" << p->id << " -> " << v->id << ";\n";
			}
			//for(auto el = v->oddChildren.getFirstElement(); el != nullptr; el = el->nxtElem) {
				//dotStream << "\t" << v->id << " -> " << el->value->id << "[color=blue,penwidth=0.5];\n";
			//}
		}
		gmlStream << "]\n";
		dotStream << "}\n";
		std::string gmlBuffer = gmlStream.str(), dotBuffer = dotStream.str();
		std::ofstream gmlWriter, dotWriter;
		std::stringstream gmlFileNameBuilder, dotFileNameBuilder;
		gmlFileNameBuilder << "debug/graphs/" << printCtr << ".gml";
		dotFileNameBuilder << "debug/graphs/" << printCtr << ".dot";
		gmlWriter.open(gmlFileNameBuilder.str());
		dotWriter.open(dotFileNameBuilder.str());
		if (!gmlWriter.fail() && !dotWriter.fail()) {
			gmlWriter << gmlBuffer;
			dotWriter << dotBuffer;
			DEBUG("Printed image " << printCtr++ << " to files " << gmlFileNameBuilder.str() << " and " << dotFileNameBuilder.str() << "\n");
		} else {
			DEBUG("Failed to print image " << printCtr++ << " to files " << gmlFileNameBuilder.str() << " and " << dotFileNameBuilder.str() << "\n");
		}
		vs->deleteStructure();
		gmlWriter.close();
		dotWriter.close();
	}

	template <class Label>
	void CherryTree<Label>::recursivePrintNode(Vertex<Label> * v, const std::string & prefix, std::stringstream & stream) {
		stream << prefix.c_str() << "-> " << v->id;
		if (v->getContainingTree() != nullptr && v->getContainingTree()->root != nullptr) {
			stream << "(" << v->getContainingTree()->root->id << ")";
		} else {
			stream << "(0)";
		}
		if (v->getMatchingPartner() != nullptr) {
			Vertex<Label>* w = v->getMatchingPartner();
			stream << " -> " << w->id;
			if (w->getContainingTree() != nullptr && w->getContainingTree()->root != nullptr) {
				stream << "(" << w->getContainingTree()->root->id << ")";
			} else {
				stream << "(0)";
			}
			stream << "\n";
			for (auto el = w->oddChildren.getFirstElement(); el != nullptr; el = el->nxtElem) {
				recursivePrintNode(el->value, prefix + "\t", stream);
			}
		} else {
			stream << "\n";
			for (auto el = v->oddChildren.getFirstElement(); el != nullptr; el = el->nxtElem) {
				recursivePrintNode(el->value, prefix + "\t", stream);
			}
		}
	}
}