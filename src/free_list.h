#ifndef FREEBLOCKSALLOCATOR_H
#define FREEBLOCKSALLOCATOR_H

#include <iostream>
#include "allocator.h"

class free_list_alloc : public Allocator {
public:
	void* buffer_begin = nullptr; // Указатель на начало буффера
	void* buffer_end = nullptr; // Указатель на конец буффера
	explicit free_list_alloc(size_t total_size);
	~free_list_alloc() {
		::free(buffer_begin);
	}
	struct Header { // Информация о блоке
		public:
		size_t size; // Размер блока
		size_t prev_size; // Размер предыдущего блока
		bool is_free;
		inline Header *next() {
			return (Header *) ((char *) (this + 1) + size);
		}
		inline Header *previous() {
			return (Header *) ((char *) this - prev_size) - 1;
		}
	};
	Header* find_block(size_t size) { // Поиск наиболее подходящего блока
		auto header = static_cast<Header *>(buffer_begin);
		auto best_header = static_cast<Header *>(buffer_end);
		size_t min_size = m_total_size;
		while (header < buffer_end) {
			if (header->is_free && header->size >= size) {
				if (min_size > header->size) {
					min_size = header->size;
					best_header = header;
				}
			}
			header = header->next();
		}
		if (best_header == (Header*)buffer_end) {
			return {nullptr};
		}
		return best_header;
	}
	void split(Header *header, size_t chunk) { // Отделяет блок
		size_t block_size = header->size;
		header->size = chunk;
		header->is_free = false;
		if (block_size - chunk >= sizeof(Header)) {
			auto *next = header->next();
			next->prev_size = chunk;
			next->size = block_size - chunk - sizeof(Header);
			next->is_free = true;
			m_used += chunk + sizeof(Header);
			auto *next_next = next->next();
			if (next_next < buffer_end) {
				next_next->prev_size = next->size;
			}
		} 
		else {
			header->size = block_size;
			m_used += block_size;
		}
	}
	bool validate(void *ptr) { // Корректность указателя
		auto *header = static_cast<Header *>(buffer_begin);
		while (header < buffer_end) {
		if (header + 1 == ptr){ return true; }
			header = header->next();
		}
		return false;
	}
public:
	virtual void* Allocate(std::size_t new_size) override;
	virtual void Free(void* ptr) override;
	virtual void Dumb() override;
private:
	virtual void reset() override;
	void merge(Header *header);
};

#endif
