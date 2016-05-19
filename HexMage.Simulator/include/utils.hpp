#ifndef HEXMAGE_SIM_UTILS_HPP
#define HEXMAGE_SIM_UTILS_HPP

template <typename T>
class Index
{
	std::vector<T>& v_;
	std::size_t index_;
public:
	Index(std::vector<T>& v, std::size_t index) :
		v_(v), index_(index) {}

	T& get() { return v_[index_]; }
	const T& get() const { return v_[index_]; }

	T& operator*() { return get(); }
	const T& operator*() const { return get(); }

	operator T&() { return get(); }
	operator const T&() const { return get(); }

	T* operator->() { return &get(); }
	const T* operator->() const { return &get(); }

	bool operator==(const Index& rhs) const { return get() == rhs.get(); }
	bool operator!=(const Index& rhs) const { return get() != rhs.get(); }
};


#endif
