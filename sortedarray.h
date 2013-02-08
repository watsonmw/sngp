#ifndef SORTEDARRAY_H
#define SORTEDARRAY_H

// Fixed sized smallish sorted array.
// Elements that are added are automatically sorted.
// We could do binary search when looking for insertion
// point for a slight speedup (disagree? profile it!)
template <typename T>
class SortedArray
{
public:
    SortedArray(int maxSize)
      : _p(0),
        _size(0),
        _maxSize(maxSize)
    {
        _p = new T[maxSize]();
    }

    ~SortedArray() {
        delete [] _p; _p = 0;
    }

    T* data() const { return _p; }
    int size() const { return _size; }

    bool add(T val) {
        int i = 0;
        for (; i < _size; ++i) {
            int tmp = _p[i];
            if (tmp > val) {
                for (int j = _size; j > i; --j) {
                     _p[j] = _p[j-1];
                }
                break;
            } else if (tmp == val) {
                return false;
            }
        }
        _p[i] = val;
        _size++;
        return true;
    }

    void clear() { _size = 0; }

private:
    T* _p;
    int _size;
    int _maxSize;
};

#endif // SORTEDARRAY_H
