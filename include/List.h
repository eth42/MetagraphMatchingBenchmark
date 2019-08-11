#pragma once

namespace maxmatching {
	template<class T>
	class List;
}

#include "ListElement.h"

namespace maxmatching {
	/* Basic implementation of a doubly connected list */
	template<class T>
	class List {
	private:
		ListElement<T>* firstElem;
		ListElement<T>* lastElem;
		unsigned int size;
	public:
		List();
		~List();

		void push(T* elem);
		void push(ListElement<T>* elem);
		void append(T* elem);
		void append(ListElement<T>* elem);
		T* pop();
		ListElement<T>* popElem();
		void remove(ListElement<T>* elem);
		void clear();

		void deleteStructure();
		void deleteFull();

		ListElement<T>* getFirstElement();
		ListElement<T>* getLastElement();
		unsigned int getSize();
		bool isEmpty();
	};

}

#include "List.tpp"
