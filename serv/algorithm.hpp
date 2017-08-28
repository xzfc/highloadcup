template <class T, class Cmp>
void insert_sorted(std::vector<T> &vec, const T &item, const Cmp &cmp)
{
    vec.insert(std::upper_bound(vec.begin(), vec.end(), item, cmp), item);
}

template <class T, class Cmp>
void remove_sorted(std::vector<T> &vec, const T &item, const Cmp &cmp)
{
    auto it = std::equal_range(vec.begin(), vec.end(), item, cmp);
    vec.erase(it.first, it.second);
}

template <class T, class V, class Cmp>
typename std::vector<T>::iterator
binary_find(std::vector<T> &vec, const V &val, const Cmp &cmp) {
    auto it = std::lower_bound(std::begin(vec), std::end(vec), val, cmp);
    if (it == std::end || cmp(val, *it))
	    return std::end(vec);
    return it;
}

template <class It>
void reorder(const It &pos, const It &to) {
	auto old = *pos;
	if (pos < to) {
		std::copy(pos+1, to, pos);
		*(to-1) = old;
	} else {
		std::copy_backward(to, pos, pos+1);
		*to = old;
	}
}
