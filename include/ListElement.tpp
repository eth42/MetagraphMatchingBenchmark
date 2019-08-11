#include "ListElement.h"

namespace maxmatching {
	template<class T>
	ListElement<T>::ListElement()
		: containingList(nullptr)
		, prvElem(nullptr)
		, nxtElem(nullptr)
		, value(nullptr) {}

	template<class T>
	ListElement<T>::ListElement(T* value)
		: containingList(nullptr)
		, prvElem(nullptr)
		, nxtElem(nullptr)
		, value(value) {}


	template<class T>
	ListElement<T>::~ListElement() {
		if (this->containingList != nullptr) {
			this->containingList->remove(this);
		}
	}

	template<class T>
	void ListElement<T>::deleteFull() {
		delete(this->value);
		delete(this);
	}
}