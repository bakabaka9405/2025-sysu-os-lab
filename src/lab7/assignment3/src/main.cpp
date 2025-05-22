#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class LRUCache {
	int capacity;		   // 页表大小
	vector<int> pageTable; // 存储页号，-1表示空
	vector<int> recent;	   // 记录每个页的最近访问值

public:
	explicit LRUCache(int capacity)
		: capacity(capacity), pageTable(capacity, -1), recent(capacity, 0) {}

	// 请求页面，返回是否命中和被替换的页（-1表示无替换）
	pair<bool, int> request(int page) {
		for (auto& i : recent) i++;
		int max_recent = -1, index = -1;
		for (int i = 0; i < capacity; i++) {
			if (pageTable[i] == page) {
				recent[i] = 0; // 命中，重置最近访问值
				return { true, -1 };
			}
			if (recent[i] > max_recent) max_recent = recent[i], index = i;
		}
		int replaced = std::exchange(pageTable[index], page); // 替换页
		recent[index] = 0;									  // 重置最近访问值
		return { false, replaced };
	}

	// 打印当前页表
	void printTable() {
		cout << "Page Table: " << endl;
		for (int i = 0; i < capacity; ++i) {
			cout << "Page " << i << " = " << pageTable[i] << " Recent= " << recent[i] << "\n";
		}
		cout << endl;
	}
};

int main() {
	int lru_size, query_count;
	cin >> lru_size >> query_count;
	LRUCache lru(lru_size);
	for (int i = 0; i < query_count; i++) {
		int page;
		cin >> page;
		auto&& [hit, replaced] = lru.request(page);
		if (hit) {
			cout << "Page " << page << " hit\n";
		}
		else {
			cout << "Page " << page << " miss, replaced page " << replaced << "\n";
		}
	}
	lru.printTable();
	return 0;
}