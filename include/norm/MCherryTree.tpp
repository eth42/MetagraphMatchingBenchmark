#include "norm/MCherryTree.h"

namespace maxmatching {
namespace norm {
	template <class Label>
	MCherryTree<Label>::MCherryTree(MVertex<Label>* root)
		: consistent(true)
		, root(root)
		, size(0)
		, metaVertex(new MVertex<Label>(this))
		, listElem(this) {
		root->setContainingTree(this);
		Statistics::incrementTreeCreated();
	}

	template <class Label>
	MCherryTree<Label>::~MCherryTree() {
		DEBUG("Deleting tree " << (this->root == nullptr ? 0 : this->root->id) << "@" << this << "\n");
		this->printTree();
		/* Prevent any prints during deletion */
		consistent = false;
		/* Remove all vertices from the tree, so the vertices can't reference the tree anymore. */
		auto containedVertices = this->getVertices();
		while (!containedVertices->isEmpty()) {
			MVertex<Label>* v = containedVertices->pop();
			v->setContainingTree(nullptr);
		}
		containedVertices->deleteStructure();
		/* Delete the metavertex representing this tree */
		if (this->metaVertex != nullptr) {
			this->metaVertex->reset();
			this->metaVertex->clearEdges();
			delete(this->metaVertex);
		}
		Statistics::incrementTreeDeleted();
	}

	/* Rotates the tree according to the pseudocode, except we're
	 * using HalfEdges instead of vertices in the lists here.
	 * The adaptation is mostly trivial though. */
	template <class Label>
	void MCherryTree<Label>::rotate(MVertex<Label>* newRoot) {
		DEBUG("Rotating tree " << this->root->id << " to " << newRoot->id << "\n");
		if (this->root == newRoot) {
			return;
		}
#ifdef DEBUG_F
		if (newRoot->getContainingBlossom() != nullptr) {
			DEBUG("MVertex " << newRoot->id << " is in blossom " << newRoot->getContainingBlossom()->getReceptacle()->id << "\n");
		}
#endif
		this->printTree();
		bool consistencyUpdate = this->consistent;
		this->consistent = false;
		List<HalfEdge<Label>>* l = new List<HalfEdge<Label>>();
		MCherryBlossom<Label>* blossom;
		MVertex<Label>* u = newRoot;
		while (u != this->root) {
			blossom = u->getContainingBlossom();
			while (blossom != nullptr) {
				u = blossom->getReceptacle();
				blossom = u->getContainingBlossom();
			}
			HalfEdge<Label>* me = u->getMatchingPartnerEdge();
			/* Only the root has no matching partner, thus this equals the break */
			if (me != nullptr) {
				l->append(me);
				MVertex<Label>* s = me->end;
				l->append(s->getEvenParentEdge());
				u = s->getEvenParent();
			}
		}
#ifdef DEBUG_F
		DEBUG("Collected edges: ");
		for (auto el = l->getFirstElement(); el != nullptr; el = el->nxtElem) {
			DEBUG("(" << el->value->start->id << ", " << el->value->end->id << "), ");
		}
		DEBUG("\n");
#endif
		List<MCherryBlossom<Label>> rotateBlossoms;
		List<MVertex<Label>> newReceptacles;
		u = newRoot;
		blossom = newRoot->getContainingBlossom();
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
			HalfEdge<Label>* e1 = l->pop();
			HalfEdge<Label>* e2 = l->pop();
			MVertex<Label>* u = e1->start;
			MVertex<Label>* s = e2->start;
			MVertex<Label>* t = e2->end;
			s->setEvenParent(e1->inverse);
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
			s->setMatchingPartner(e2);
			t->setMatchingPartner(e2->inverse);
		}
		l->deleteStructure();
		newRoot->setMatchingPartner(nullptr);
		this->root = newRoot;
		/* Since trees are immediately destroyed after rotating, we don't need
		 * to update the levels. Consistency of the tree is not an issue.
		 * If anyone changes the code to use the trees after any rotation, this has to
		 * be commented back in! */
		 //this->updateLevel();
		this->consistent = consistencyUpdate;
		this->printTree();
	}

	/* This is to be called, if the receptacle has just been added and the other vertices
	* of the corolla are in no tree yet. It claims the corolla for this tree and manages
	* the children of the new nodes. */
	template <class Label>
	void MCherryTree<Label>::adoptBlossom(MCherryBlossom<Label> * blossom) {
		DEBUG("Adopting blossom " << blossom->getReceptacle()->id << "\n");
		bool consistencyUpdate = this->consistent;
		this->consistent = false;
		auto fun = [this, blossom](MVertex<Label> * v)->void {
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
				HalfEdge<Label>* e = elem->value;
				MVertex<Label>* child = e->end;
				if (child->getContainingBlossom() != blossom) {
					this->add(e);
				}
			}
		};
		blossom->updateLevel();
		blossom->foreachVertex(fun);
		this->consistent = consistencyUpdate;
	}

	/* Adds a vertex into the tree along the given edge.
	 * Also uses as much of potential old tree structure as possible
	 * given the information stored in the vertices. */
	template <class Label>
	void MCherryTree<Label>::add(HalfEdge<Label> * parentToChild) {
#ifdef DEBUG_F
		MVertex<Label>* parent = parentToChild->start;
#endif
		MVertex<Label>* child = parentToChild->end;
		/* Sanity check */
		if (child->getContainingTree() != nullptr) {
			return;
		}
		bool consistencyUpdate = this->consistent;
		this->consistent = false;
		/* Update references for containment in tree */
		MVertex<Label>* newPartner = child->getMatchingPartner();
		DEBUG("Adding vertices "
			<< (parent == nullptr ? 0 : parent->id)
			<< " -> "
			<< (child == nullptr ? 0 : child->id)
			<< " -> "
			<< (newPartner == nullptr ? 0 : newPartner->id) << "\n");
		child->setEvenParent(parentToChild->inverse);
		child->setContainingTree(this);
		newPartner->setContainingTree(this);
		newPartner->enqueue();
		this->size += 2;
		/* Remove all odd children of the new node, since odd and non-even nodes can't have odd children */
		while (!child->oddChildren.isEmpty()) {
			auto elem = child->oddChildren.popElem();
			elem->value->end->setEvenParent(nullptr);
		}
		/* If this gets added, the old blossom should be dead for consistency reasons */
		if (child->getContainingBlossom() != nullptr) {
			delete(child->getContainingBlossom());
		}
		/* Delete potentially borne blossoms by now odd and non-even node */
		while (!child->bearingBlossoms.isEmpty()) {
			delete(child->bearingBlossoms.popElem()->value);
		}
		/* Adopt blossoms borne by the matching partner, which is even in this tree */
		for (auto el = newPartner->bearingBlossoms.getFirstElement(); el != nullptr; el = el->nxtElem) {
			/* getReference should be redundant here, since we're looking
			 * at the representant anyways, but just to be sure... */
			this->adoptBlossom(el->value->getReference());
		}
		/* Call recursive, if this MVertex<Label> was hosting an old twig */
		DEBUG("Node " << newPartner->id << " has " << newPartner->oddChildren.getSize() << " odd children\n");
		for (auto elem = newPartner->oddChildren.getFirstElement(); elem != nullptr; elem = elem->nxtElem) {
			HalfEdge<Label>* e = elem->value;
			MVertex<Label>* child = e->end;
			DEBUG("Node " << newPartner->id << " considers child " << child->id << "\n");
			MCherryBlossom<Label> * childBlossom = child->getContainingBlossom();
			if (childBlossom == nullptr || childBlossom->getReceptacle() != newPartner) {
				/* Otherwise this should've been handled by adopting the blossom */
				if (child->getContainingTree() != this) {
					this->add(e);
				}
			}
		}
		/* Safe upwards in the trees */
		HalfEdge<Label>* pe = newPartner->getEvenParentEdge();
		if (pe != nullptr) {
			MVertex<Label>* oldParent = pe->end;
			newPartner->setEvenParent(nullptr);
			if (oldParent->getContainingTree() != newPartner->getContainingTree()) {
				this->add(pe);
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
	void MCherryTree<Label>::remove(MVertex<Label> * v) {
		bool consistencyUpdate = this->consistent;
		this->consistent = false;
		if (v->getContainingTree() == this) {
			v->setContainingTree(nullptr);
			if (v == this->root) {
				this->root = nullptr;
			}
			MVertex<Label>* m = v->getMatchingPartner();
			this->size--;
			if (m != nullptr) {
				this->remove(m);
			}
			for (auto el = v->oddChildren.getFirstElement(); el != nullptr; el = el->nxtElem) {
				this->remove(el->value->end);
			}
		}
		this->consistent = consistencyUpdate;
	}

	/* Checks if a blossom creation is necessary and if so performs it
	 * according to the pseudocode. */
	template <class Label>
	void MCherryTree<Label>::makeBlossom(HalfEdge<Label> * e) {
		MVertex<Label>* u = e->start;
		MVertex<Label>* w = e->end;
		DEBUG("Checking blossom creation between " << u->id << " and " << w->id << "\n");
		MCherryBlossom<Label> * uBlossom = u->getContainingBlossom();
		MCherryBlossom<Label> * wBlossom = w->getContainingBlossom();
		MCherryBlossom<Label> * blossom;
		/* Return condition: Both vertices are in the same blossom.
		* Either they are in the same corolla
		* or one of the vertices is the receptacle for the blossom
		* of which the other is in the corolla */
		if ((uBlossom != nullptr && uBlossom == wBlossom)
			|| (uBlossom != nullptr && uBlossom->getReceptacle() == w)
			|| (wBlossom != nullptr && wBlossom->getReceptacle() == u)) {
			return;
		}
#ifdef DEBUG_F
		DEBUG("Vertices upward from u: ");
		auto ul = u->createEvenPathToRoot();
		for (auto el = ul->getFirstElement(); el != nullptr; el = el->nxtElem) {
			auto v = el->value->end;
			auto m = v->getMatchingPartner();
			auto p = v->getEvenParent();
			DEBUG(v->id << "(" << (p == nullptr ? 0 : p->id) << "), ");
		}
		ul->deleteStructure();
		DEBUG("\nVertices upward from w: ");
		auto uw = w->createEvenPathToRoot();
		for (auto el = uw->getFirstElement(); el != nullptr; el = el->nxtElem) {
			auto v = el->value->end;
			auto m = v->getMatchingPartner();
			auto p = v->getEvenParent();
			DEBUG(v->id << "(" << (p == nullptr ? 0 : p->id) << "), ");
		}
		uw->deleteStructure();
		DEBUG("\n");
#endif
		this->printTree();
		bool consistencyUpdate = this->consistent;
		this->consistent = false;
		/* Using the level of the vertices, the search for a common even
		 * ancestor is performed by stepwise creation of the paths from the
		 * vertex with the higher level. In the worst case, this results in
		 * the same result as the pseudocode, creating paths to the root of
		 * the tree, but on average, this ends up at the demanded vertex. */
		MVertex<Label> * s = u;
		MVertex<Label> * t = w;
		MCherryBlossom<Label> * possibleReuseBlossom = nullptr;
		std::vector<MVertex<Label>*> ss;
		ss.push_back(s);
		std::vector<MVertex<Label>*> ts;
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
			MVertex<Label>* sb = ss.back();
			MCherryBlossom<Label>* blossom = sb->getContainingBlossom();
			if (blossom != nullptr) {
				possibleReuseBlossom = blossom;
			}
		}
		if (ts.size() > 0) {
			MVertex<Label>* tb = ts.back();
			MCherryBlossom<Label>* blossom = tb->getContainingBlossom();
			if (blossom != nullptr) {
				possibleReuseBlossom = blossom;
			}
		}
		ss.clear();
		ts.clear();
		MVertex<Label>* r = s;
		MCherryBlossom<Label>* newBlossom;
		/* Try to reuse an existing blossom, through which one of both pathes must have gone */
		if (possibleReuseBlossom != nullptr && possibleReuseBlossom->getReceptacle() == r) {
			newBlossom = possibleReuseBlossom;
			DEBUG("Reusing blossom with receptacle " << r->id << " @ " << newBlossom << "\n");
		} else {
			newBlossom = new MCherryBlossom<Label>(r);
			DEBUG("New blossom with receptacle " << r->id << " @ " << newBlossom << "\n");
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
						MVertex<Label>* r2 = blossom->getReceptacle();
						if (r != r2) {
							while (s != r2) {
								HalfEdge<Label>* smpe = s->getMatchingPartner()->getEvenParentEdge();
								MVertex<Label>* t = smpe->end;
								/* The recepticle might actually be part of the new blossom in which case it is not to be modified */
								if (t->getContainingBlossom() != newBlossomRef) {
									t->setEvenParent(smpe->inverse);
								}
								s = t;
							}
						} else {
							s = r;
						}
						newBlossomRef->merge(blossom);
						newBlossomRef = newBlossom->getReference();
					} else {
						s = r;
					}
				} else {
					newBlossom->add(s);
					newBlossom->add(s->getMatchingPartner());
					HalfEdge<Label>* smpe = s->getMatchingPartner()->getEvenParentEdge();
					MVertex<Label>* t = smpe->end;
					if (t != r && t->getContainingBlossom() != newBlossomRef) {
						t->setEvenParent(smpe->inverse);
					}
					s = t;
				}
			}
		}
		if (!uFreeze) {
			u->setEvenParent(e);
		}
		if (!wFreeze) {
			w->setEvenParent(e->inverse);
		}
		this->consistent = consistencyUpdate;
		this->printTree();
	}

	/* Updates the level of all vertices */
	template <class Label>
	void MCherryTree<Label>::updateLevel() {
		this->updateLevelBelow(this->root);
	}

	/* Updates the level of all vertices below the given vertex */
	template <class Label>
	void MCherryTree<Label>::updateLevelBelow(MVertex<Label> * v) {
		List<MVertex<Label>>* remainingVertices = new List<MVertex<Label>>();
		remainingVertices->push(v);
		while (!remainingVertices->isEmpty()) {
			MVertex<Label>* w = remainingVertices->pop();
			w->updateLevel();
			for (auto childEl = w->oddChildren.getFirstElement(); childEl != nullptr; childEl = childEl->nxtElem) {
				HalfEdge<Label>* e = childEl->value;
				MVertex<Label>* m = e->end->getMatchingPartner();
				remainingVertices->append(m);
			}
		}
		remainingVertices->deleteStructure();
	}

	template <class Label>
	List<MVertex<Label>>* MCherryTree<Label>::getVertices() {
		List<MVertex<Label>>* ret = new List<MVertex<Label>>();
		if (this->root == nullptr) {
			return ret;
		}
		ret->append(this->root);
		for (auto el = ret->getFirstElement(); el != nullptr; el = el->nxtElem) {
			MVertex<Label>* v = el->value;
			for (auto el2 = v->oddChildren.getFirstElement(); el2 != nullptr; el2 = el2->nxtElem) {
				HalfEdge<Label>* e = el2->value;
				MVertex<Label>* w = e->end;
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
	unsigned int MCherryTree<Label>::printCtr = 0;

	template <class Label>
	void MCherryTree<Label>::logTree() {
		std::stringstream stream;
		recursivePrintNode(root, "", stream);
		std::string buffer = stream.str();
		DEBUG(buffer);
	}

	/* Prints the tree for debugging purposes. This is disabled if not using debugging. */
	template <class Label>
	void MCherryTree<Label>::printTree() {
#ifndef DEBUG_F
		return;
#else
		if (!this->consistent || !DEBUG_ENABLED) {
			return;
		}
		DEBUG("@Tree(" << (this->root == nullptr ? 0 : this->root->id) << ") ");
		std::stringstream gmlStream, dotStream;
		gmlStream << "graph [\n";
		dotStream << "digraph G {\n";
		auto vs = this->getVertices();
		for (auto it = vs->getFirstElement(); it != nullptr; it = it->nxtElem) {
			MVertex<Label>* v = it->value;
			gmlStream << "\tnode [\n\t\tid " << v->id << "\n\t\tlabel \"" << v->id << "\"\n\t]\n";
		}
		for (auto it = vs->getFirstElement(); it != nullptr; it = it->nxtElem) {
			MVertex<Label>* v = it->value;
			MVertex<Label>* m = v->getMatchingPartner();
			MVertex<Label>* p = v->getEvenParent();
			if (m != nullptr) {
				gmlStream << "\tedge [\n\t\tsource " << v->id << "\n\t\ttarget " << m->id << "\n\t\tlabel \"m\"\n\t]\n";
				dotStream << "\t" << v->id << " -> " << m->id << "[color=red,penwidth=2.0];\n";
			}
			if (p != nullptr) {
				gmlStream << "\tedge [\n\t\tsource " << p->id << "\n\t\ttarget " << v->id << "\n\t]\n";
				dotStream << "\t" << p->id << " -> " << v->id << ";\n";
			}
			//for(auto el = v->oddChildren.getFirstElement(); el != nullptr; el = el->nxtElem) {
				//dotStream << "\t" << v->id << " -> " << el->value->end->id << "[color=blue,penwidth=0.5];\n";
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
#endif
	}

	template <class Label>
	void MCherryTree<Label>::recursivePrintNode(MVertex<Label> * v, const std::string & prefix, std::stringstream & stream) {
		stream << prefix.c_str() << "-> " << v->id;
		if (v->getContainingTree() != nullptr && v->getContainingTree()->root != nullptr) {
			stream << "(" << v->getContainingTree()->root->id << ")";
		} else {
			stream << "(0)";
		}
		if (v->getMatchingPartner() != nullptr) {
			MVertex<Label>* w = v->getMatchingPartner();
			stream << " -> " << w->id;
			if (w->getContainingTree() != nullptr && w->getContainingTree()->root != nullptr) {
				stream << "(" << w->getContainingTree()->root->id << ")";
			} else {
				stream << "(0)";
			}
			stream << "\n";
			for (auto el = w->oddChildren.getFirstElement(); el != nullptr; el = el->nxtElem) {
				HalfEdge<Label>* e = el->value;
				recursivePrintNode(e->end, prefix + "\t", stream);
			}
		} else {
			stream << "\n";
			for (auto el = v->oddChildren.getFirstElement(); el != nullptr; el = el->nxtElem) {
				HalfEdge<Label>* e = el->value;
				recursivePrintNode(e->end, prefix + "\t", stream);
			}
		}
	}
}
}
