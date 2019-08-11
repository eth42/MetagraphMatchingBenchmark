#pragma once

namespace maxmatching {
	template<class T>
	class List;

	/* Basic implementation of elements in a doubly connected list */
	template<class T>
	class ListElement {
	public:
		List<T>* containingList;
		ListElement<T>* prvElem;
		ListElement<T>* nxtElem;
		T* value;

		ListElement();
		ListElement(T* value);
		~ListElement();

		void deleteFull();
	};
}

#include "ListElement.tpp"