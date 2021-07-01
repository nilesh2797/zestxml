// Code adopted fom https://web.stanford.edu/class/archive/cs/cs106b/cs106b.1138/materials/cppdoc/vector-h.html

/*
 * File: SparseVector.h
 * --------------
 * This file exports the SparseVector class, which represents a 1-dimensional sparse vector of (index,value) pairs
 */

#ifndef _sparse_vector_h
#define _sparse_vector_h

#include <iterator>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <algorithm>

using namespace std;

// Added by Yash
void error( std::string msg )
{
    std::cerr << msg << std::endl;
}


/*
 * Class: SparseVector<ValueType>
 * ------------------------
 * TO-DO: CHANGE COMMENT
 * This class stores an ordered list of values similar to an array.  It
 * supports traditional array selection using square brackets, but also
 * supports inserting and deleting elements.  It is similar in function to
 * the STL vector type, but is simpler both to use and to implement.
 */

template <typename ValueType>
class SparseVector {

public:
typedef pair<int,ValueType> PValueType;
/*
 * Constructor: SparseVector
 * Usage: SparseVector<ValueType> vec;
 *        SparseVector<ValueType> vec(n, value);
 * ---------------------------------------
 * Initializes a new vector.  The default constructor creates an empty
 * vector.  The second form creates an array with n elements, each of which
 * is initialized to value; if value is missing, the elements are
 * initialized to the default value for the type.
 */

   SparseVector();
   explicit SparseVector(int n, PValueType value = PValueType());
   explicit SparseVector(int n, PValueType* value_ptr);

/*
 * Destructor: ~SparseVector
 * -------------------
 * Frees any heap storage allocated by this vector.
 */

   virtual ~SparseVector();

/*
 * Method: size
 * Usage: int nElems = vec.size();
 * -------------------------------
 * Returns the number of elements in this vector.
 */

   int size() const;

/*
 * Method: isEmpty
 * Usage: if (vec.isEmpty()) ...
 * -----------------------------
 * Returns true if this vector contains no elements.
 */

   bool isEmpty() const;

/*
 * Method: clear
 * Usage: vec.clear();
 * -------------------
 * Removes all elements from this vector.
 */

   void clear();

/*
 * Method: get
 * Usage: PValueType val = vec.get(index);
 * --------------------------------------
 * Returns the element at the specified index in this vector.  This method
 * signals an error if the index is not in the array range.
 */

   const PValueType & get(int index) const;

/*
 * Method: set
 * Usage: vec.set(index, value);
 * -----------------------------
 * Replaces the element at the specified index in this vector with a new
 * value.  The previous value at that index is overwritten.  This method
 * signals an error if the index is not in the array range.
 */

   void set(int index, const PValueType & value);

/*
 * Method: insert
 * Usage: vec.insert(0, value);
 * ----------------------------
 * Inserts the element into this vector before the specified index.  All
 * subsequent elements are shifted one position to the right.  This method
 * signals an error if the index is outside the range from 0 up to and
 * including the length of the vector.
 */

   void insert(int index, PValueType value);

/*
 * Method: remove
 * Usage: vec.remove(index);
 * -------------------------
 * Removes the element at the specified index from this vector.  All
 * subsequent elements are shifted one position to the left.  This method
 * signals an error if the index is outside the array range.
 */

   void remove(int index);

/*
 * Method: add
 * Usage: vec.add(value);
 * ----------------------
 * Adds a new value to the end of this vector.  To ensure compatibility
 * with the vector class in the Standard Template Library, this method is
 * also called push_back.
 */

   void add(PValueType value);
   void push_back(PValueType value);

/*
 * Operator: []
 * Usage: vec[index]
 * -----------------
 * Overloads [] to select elements from this vector.  This extension
 * enables the use of traditional array notation to get or set individual
 * elements.  This method signals an error if the index is outside the
 * array range.  The file supports two versions of this operator, one for
 * const vectors and one for mutable vectors.
 */

   PValueType & operator[](int index);
   const PValueType & operator[](int index) const;

/*
 * Operator: +
 * Usage: v1 + v2
 * --------------
 * Concatenates two vectors.
 */

   SparseVector operator+(const SparseVector & v2) const;

/*
 * Operator: +=
 * Usage: v1 += v2;
 *        v1 += value;
 * -------------------
 * Adds all of the elements from v2 (or the single specified value) to v1. 
 * As a convenience, the SparseVector package also overloads the comma operator
 * so that it is possible to initialize a vector like this:
 *
 *    SparseVector<int> digits;
 *    digits += 0, 1, 2, 3, 4, 5, 6, 7, 8, 9;
 */

   SparseVector & operator+=(const SparseVector & v2);
   SparseVector & operator+=(const PValueType & value);

/*
 * Method: toString
 * Usage: string str = vec.toString();
 * -----------------------------------
 * Converts the vector to a printable string representation.
 */

   std::string toString();

/*
 * Method: mapAll
 * Usage: vec.mapAll(fn);
 * ----------------------
 * Calls the specified function on each element of the vector in ascending
 * index order.
 */

   void mapAll(void (*fn)(PValueType)) const;
   void mapAll(void (*fn)(const PValueType &)) const;

   template <typename FunctorType>
   void mapAll(FunctorType fn) const;

// Added by Yash
/*
 * Method: sort
 * Usage: vec.sort(0,true);
 * -------------------------------
 * Sorts the data pairs in either ascending (true) or descending (false) orders by either comparing index (0) or value (1) elements
 */

   void sort( int element, bool sort_type )
   {
      if( element!=0 && element!=1 )
         error( "sort: Invalid first argument" );

      if( element==0 )
      {
         if( sort_type )
            std::sort( elements, elements+count, []( const PValueType& a, const PValueType& b) -> bool { return a.first < b.first; } );
         else
            std::sort( elements, elements+count, []( const PValueType& a, const PValueType& b) -> bool { return a.first > b.first; } );
      }
      else
      {
         if( sort_type )
            std::sort( elements, elements+count, []( const PValueType& a, const PValueType& b) -> bool { return a.second < b.second; } );
         else
            std::sort( elements, elements+count, []( const PValueType& a, const PValueType& b) -> bool { return a.second > b.second; } );
      }
   }

/*
 * Additional SparseVector operations
 * ----------------------------
 * In addition to the methods listed in this interface, the SparseVector class
 * supports the following operations:
 *
 *   - Stream I/O using the << and >> operators
 *   - Deep copying for the copy constructor and assignment operator
 *   - Iteration using the range-based for statement or STL iterators
 *
 * The iteration forms process the SparseVector in index order.
 */

/* Private section */

/**********************************************************************/
/* Note: Everything below this point in the file is logically part    */
/* of the implementation and should not be of interest to clients.    */
/**********************************************************************/

private:

/*
 * Implementation notes: SparseVector data structure
 * -------------------------------------------
 * The elements of the SparseVector are stored in a dynamic array of the
 * specified element type.  If the space in the array is ever exhausted,
 * the implementation doubles the array capacity.
 */

/* Instance variables */

   PValueType *elements;        /* A dynamic array of the elements   */
   int capacity;               /* The allocated size of the array   */
   int count;                  /* The number of elements in use     */
   bool owns_data;

/* Private methods */

   void expandCapacity();
   void deepCopy(const SparseVector & src);

/*
 * Hidden features
 * ---------------
 * The remainder of this file consists of the code required to support deep
 * copying and iteration.  Including these methods in the public interface
 * would make that interface more difficult to understand for the average
 * client.
 */

public:

/*
 * Deep copying support
 * --------------------
 * This copy constructor and operator= are defined to make a deep copy,
 * making it possible to pass or return vectors by value and assign from
 * one vector to another.
 */

   SparseVector(const SparseVector & src);
   SparseVector & operator=(const SparseVector & src);

/*
 * Operator: ,
 * -----------
 * Adds an element to the vector passed as the left-hand operatand.  This
 * form makes it easier to initialize vectors in old versions of C++.
 */

   SparseVector & operator,(const PValueType & value);

/*
 * Iterator support
 * ----------------
 * The classes in the StanfordCPPLib collection implement input iterators
 * so that they work symmetrically with respect to the corresponding STL
 * classes.
 */

   class iterator :
      public std::iterator<std::random_access_iterator_tag, PValueType> {

   private:
      const SparseVector *vp;
      int index;

   public:

      iterator() {
         this->vp = NULL;
      }

      iterator(const iterator & it) {
         this->vp = it.vp;
         this->index = it.index;
      }

      iterator(const SparseVector *vp, int index) {
         this->vp = vp;
         this->index = index;
      }

      iterator & operator++() {
         index++;
         return *this;
      }

      iterator operator++(int) {
         iterator copy(*this);
         operator++();
         return copy;
      }

      iterator & operator--() {
         index--;
         return *this;
      }

      iterator operator--(int) {
         iterator copy(*this);
         operator--();
         return copy;
      }

      bool operator==(const iterator & rhs) {
         return vp == rhs.vp && index == rhs.index;
      }

      bool operator!=(const iterator & rhs) {
         return !(*this == rhs);
      }

      bool operator<(const iterator & rhs) {
         //extern void error(std::string msg);
         if (vp != rhs.vp) error("Iterators are in different vectors");
         return index < rhs.index;
      }

      bool operator<=(const iterator & rhs) {
         //extern void error(std::string msg);
         if (vp != rhs.vp) error("Iterators are in different vectors");
         return index <= rhs.index;
      }

      bool operator>(const iterator & rhs) {
         //extern void error(std::string msg);
         if (vp != rhs.vp) error("Iterators are in different vectors");
         return index > rhs.index;
      }

      bool operator>=(const iterator & rhs) {
         //extern void error(std::string msg);
         if (vp != rhs.vp) error("Iterators are in different vectors");
         return index >= rhs.index;
      }

      iterator operator+(const int & rhs) {
         return iterator(vp, index + rhs);
      }

      iterator operator+=(const int & rhs) {
         index += rhs;
         return *this;
      }

      iterator operator-(const int & rhs) {
         return iterator(vp, index - rhs);
      }

      iterator operator-=(const int & rhs) {
         index -= rhs;
         return *this;
      }

      int operator-(const iterator & rhs) {
         //extern void error(std::string msg);
         if (vp != rhs.vp) error("Iterators are in different vectors");
         return index - rhs.index;
      }

      PValueType & operator*() {
         return vp->elements[index];
      }

      PValueType *operator->() {
         return &vp->elements[index];
      }

      PValueType & operator[](int k) {
         return vp->elements[index + k];
      }

   };

   iterator begin() const {
      return iterator(this, 0);
   }

   iterator end() const {
      return iterator(this, count);
   }
};

/* Implementation section */

//extern void error(std::string msg);

/*
 * Implementation notes: SparseVector constructor and destructor
 * -------------------------------------------------------
 * The constructor allocates storage for the dynamic array and initializes
 * the other fields of the object.  The destructor frees the memory used
 * for the array.
 */

template <typename ValueType>
SparseVector<ValueType>::SparseVector() {
   count = capacity = 0;
   elements = NULL;
   owns_data = true;
}

template <typename ValueType>
SparseVector<ValueType>::SparseVector(int n, PValueType value) {
   count = capacity = n;
   elements = (n == 0) ? NULL : new PValueType[n];
   for (int i = 0; i < n; i++) {
      elements[i] = value;
   }
   owns_data = true;
}

template <typename ValueType>
SparseVector<ValueType>::SparseVector(int n, PValueType* value_ptr) {
   count = capacity = n;
   elements = (n == 0) ? NULL : value_ptr;
   owns_data = false;
}

template <typename ValueType>
SparseVector<ValueType>::~SparseVector() {
   if (elements != NULL && owns_data ) delete[] elements;
}

/*
 * Implementation notes: SparseVector methods
 * ------------------------------------
 * The basic SparseVector methods are straightforward and should require no
 * detailed documentation.
 */

template <typename ValueType>
int SparseVector<ValueType>::size() const {
   return count;
}

template <typename ValueType>
bool SparseVector<ValueType>::isEmpty() const {
   return count == 0;
}

template <typename ValueType>
void SparseVector<ValueType>::clear() {
   if (elements != NULL && owns_data ) delete[] elements;
   count = capacity = 0;
   elements = NULL;
}

template <typename ValueType>
const pair<int,ValueType> & SparseVector<ValueType>::get(int index) const {
   if (index < 0 || index >= count) error("get: index out of range");
   return elements[index];
}

template <typename ValueType>
void SparseVector<ValueType>::set(int index, const pair<int,ValueType> & value) {
   if (index < 0 || index >= count) error("set: index out of range");
   elements[index] = value;
}

/*
 * Implementation notes: insert, remove, add
 * -----------------------------------------
 * These methods must shift the existing elements in the array to make room
 * for a new element or to close up the space left by a deleted one.
 */

template <typename ValueType>
void SparseVector<ValueType>::insert(int index, pair<int,ValueType> value) {
   if( !owns_data )
      error("insert: cannot mutate SparseVector having non-owned data");
   if (count == capacity) expandCapacity();
   if (index < 0 || index > count) {
      error("insert: index out of range");
   }
   for (int i = count; i > index; i--) {
      elements[i] = elements[i - 1];
   }
   elements[index] = value;
   count++;
}

template <typename ValueType>
void SparseVector<ValueType>::remove(int index) {
   if( !owns_data )
      error("remove: cannot mutate SparseVector having non-owned data");
   if (index < 0 || index >= count) error("remove: index out of range");
   for (int i = index; i < count - 1; i++) {
      elements[i] = elements[i + 1];
   }
   count--;
}

template <typename ValueType>
void SparseVector<ValueType>::add(pair<int,ValueType> value) {
   if( !owns_data )
      error("add: cannot mutate SparseVector having non-owned data");
   insert(count, value);
}

template <typename ValueType>
void SparseVector<ValueType>::push_back(pair<int,ValueType> value) {
   if( !owns_data )
      error("push_back: cannot mutate SparseVector having non-owned data");
   insert(count, value);
}

/*
 * Implementation notes: SparseVector selection
 * --------------------------------------
 * The following code implements traditional array selection using square
 * brackets for the index.
 */

template <typename ValueType>
pair<int,ValueType> & SparseVector<ValueType>::operator[](int index) {
   if (index < 0 || index >= count) error("Selection index out of range");
   return elements[index];
}
template <typename ValueType>
const pair<int,ValueType> & SparseVector<ValueType>::operator[](int index) const {
   if (index < 0 || index >= count) error("Selection index out of range");
   return elements[index];
}

template <typename ValueType>
SparseVector<ValueType> SparseVector<ValueType>::operator+(const SparseVector & v2) const {
   SparseVector<ValueType> vec = *this;
   for( int i=0; i<v2.size(); i++ ) {
      vec.add(v2[i]);
   }
   return vec;
}

template <typename ValueType>
SparseVector<ValueType> & SparseVector<ValueType>::operator+=(const SparseVector & v2) {
   if( !owns_data )
      error("operator +=: cannot mutate SparseVector having non-owned data");
   for( int i=0; i<v2.size(); i++ ) {
      *this += v2[i];
   }
   return *this;
}

template <typename ValueType>
SparseVector<ValueType> & SparseVector<ValueType>::operator+=(const pair<int,ValueType> & value) {
   if( !owns_data )
      error("operator +=: cannot mutate SparseVector having non-owned data");
   this->add(value);
   return *this;
}

template <typename ValueType>
std::string SparseVector<ValueType>::toString() {
   std::ostringstream os;
   os << *this;
   return os.str();
}

/*
 * Implementation notes: copy constructor and assignment operator
 * --------------------------------------------------------------
 * The constructor and assignment operators follow a standard paradigm, as
 * described in the associated textbook.
 */

template <typename ValueType>
SparseVector<ValueType>::SparseVector(const SparseVector & src) {
   deepCopy(src);
}

template <typename ValueType>
SparseVector<ValueType> & SparseVector<ValueType>::operator=(const SparseVector & src) {
   if (this != &src) {
      if (elements != NULL && owns_data) delete[] elements;
      deepCopy(src);
   }
   return *this;
}

template <typename ValueType>
void SparseVector<ValueType>::deepCopy(const SparseVector & src) {
   count = capacity = src.count;
   elements = (capacity == 0) ? NULL : new pair<int,ValueType>[capacity];
   for (int i = 0; i < count; i++) {
      elements[i] = src.elements[i];
   }
}

/*
 * Implementation notes: The , operator
 * ------------------------------------
 * The comma operator works adding the right operand to the vector and then
 * returning the vector by reference so that it is set for the next value
 * in the chain.
 */

template <typename ValueType>
SparseVector<ValueType> & SparseVector<ValueType>::operator,(const pair<int,ValueType> & value) {
   if( !owns_data )
      error("operator ,: cannot mutate SparseVector having non-owned data");
   this->add(value);
   return *this;
}

/*
 * Implementation notes: mapAll
 * ----------------------------
 * The various versions of the mapAll function apply the function or
 * function object to each element in ascending index order.
 */

template <typename ValueType>
void SparseVector<ValueType>::mapAll(void (*fn)(pair<int,ValueType>)) const {
   for (int i = 0; i < count; i++) {
      fn(elements[i]);
   }
}

template <typename ValueType>
void SparseVector<ValueType>::mapAll(void (*fn)(const pair<int,ValueType> &)) const {
   for (int i = 0; i < count; i++) {
      fn(elements[i]);
   }
}

template <typename ValueType>
template <typename FunctorType>
void SparseVector<ValueType>::mapAll(FunctorType fn) const {
   for (int i = 0; i < count; i++) {
      fn(elements[i]);
   }
}

/*
 * Implementation notes: expandCapacity
 * ------------------------------------
 * This function doubles the array capacity, copies the old elements into
 * the new array, and then frees the old one.
 */

template <typename ValueType>
void SparseVector<ValueType>::expandCapacity() {
   if( !owns_data )
      error("expandCapacity: cannot mutate SparseVector having non-owned data");

   capacity = std::max(1, capacity * 2);
   pair<int,ValueType> *array = new pair<int,ValueType>[capacity];
   for (int i = 0; i < count; i++) {
      array[i] = elements[i];
   }
   if (elements != NULL) delete[] elements;
   elements = array;
}

/*
 * Implementation notes: << and >>
 * -------------------------------
 * The insertion and extraction operators use the template facilities in
 * strlib.h to read and write generic values in a way that treats strings
 * specially.
 */

template <typename ValueType>
ostream & operator<<(ostream & os, const SparseVector<ValueType> & vec) {
   int len = vec.size();
   os << len;
   for (int i = 0; i < len; i++) {
    os << " " << vec[i].first << ":" << vec[i].second;
   }
   return os;
}

template <typename ValueType>
istream & operator>>(istream & is, SparseVector<ValueType> & vec) {
   int len;
   is >> len;
   vec.clear();
   char c;
   for( int i=0; i<len; i++ )
   {
      pair<int,ValueType> value;
      is >> value.first >> c >> value.second;
      vec += value;
   }
   return is;
}

typedef SparseVector<bool> SVecB;
typedef SparseVector<int> SVecI;
typedef SparseVector<float> SVecF;
typedef SparseVector<double> SVecD;

#endif