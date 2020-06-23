#pragma once
#include "Const.h"
#include <memory>

template <typename T>
class CMyStack
{
public:
	CMyStack() = default;
	CMyStack(CMyStack<T> const& stack);
	CMyStack(CMyStack<T>&& stack);
	~CMyStack();

	void Push(T const& value);
	void Pop();
	T GetTop() const;
	bool IsEmpty() const;
	void Clear();
	size_t GetSize() const;

	CMyStack<T>& operator=(CMyStack<T> const& stack);
	CMyStack<T>& operator=(CMyStack<T>&& stack);

private:
	struct Node
	{
		Node(T const& value, std::shared_ptr<Node> const& next)
			: value(value)
			, next(next)
		{
		}
		T value;
		std::shared_ptr<Node> next = nullptr;
	};

	void CopyNode(CMyStack<T> const& stack);

	size_t m_size = 0;
	std::shared_ptr<Node> m_top = nullptr;
};

template <typename T>
CMyStack<T>::CMyStack(CMyStack<T> const& stack)
{
	CopyNode(stack);
}

template <typename T>
CMyStack<T>::CMyStack(CMyStack<T>&& stack)
{
	if (!stack.IsEmpty())
	{
		m_top = stack.m_top;
		m_size = stack.m_size;
		stack.m_top = nullptr;
		stack.m_size = 0;
	}
}

template <typename T>
CMyStack<T>::~CMyStack()
{
	Clear();
}

template <typename T>
void CMyStack<T>::Push(T const& value)
{
	auto newNode = std::make_shared<Node>(value, m_top);
	m_top = newNode;
	m_size++;
}

template <typename T>
void CMyStack<T>::Pop()
{
	if (IsEmpty())
	{
		throw std::logic_error(ERROR_EMPTY_STACK);
	}

	m_top = m_top->next;
	m_size--;
}

template <typename T>
T CMyStack<T>::GetTop() const
{
	if (IsEmpty())
	{
		throw std::logic_error(ERROR_EMPTY_STACK);
	}

	return m_top->value;
}

template <typename T>
bool CMyStack<T>::IsEmpty() const
{
	return m_size == 0;
}

template <typename T>
void CMyStack<T>::Clear()
{
	while (!IsEmpty())
	{
		Pop();
	}
}

template <typename T>
size_t CMyStack<T>::GetSize() const
{
	return m_size;
}

template <typename T>
CMyStack<T>& CMyStack<T>::operator=(CMyStack<T> const& stack)
{
	if (this != &stack)
	{
		Clear();
		CopyNode(stack);
	}
	return *this;
}

template <typename T>
CMyStack<T>& CMyStack<T>::operator=(CMyStack<T>&& stack)
{
	if (this != &stack)
	{
		m_top = stack.m_top;
		m_size = stack.m_size;
		stack.m_top = nullptr;
		stack.m_size = 0;
	}

	return *this;
}

template <typename T>
void CMyStack<T>::CopyNode(CMyStack<T> const& stack)
{
	if (!stack.IsEmpty())
	{
		m_size = stack.m_size;
		std::shared_ptr<Node> pCopiedNode = stack.m_top;

		m_top = std::make_shared<Node>(*pCopiedNode);
		auto pPasteNode = m_top;

		while (pCopiedNode->next != nullptr)
		{
			pPasteNode->next = std::make_shared<Node>(*pCopiedNode->next);

			pCopiedNode = pCopiedNode->next;
			pPasteNode = pPasteNode->next;
		}
	}
}