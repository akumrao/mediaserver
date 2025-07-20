/************************************************************

This class  equilvalent to #include<boost/circular_buffer.hpp>

*****************************************************************************************************************/

#include<iostream>
#include<vector>
#include<list>

#ifndef FIXEDLIST_H
#define FIXEDLIST_H

template <typename T>
class FixedList : public std::list<T> {
public:
    
    FixedList(int max)
    {
        MaxLen = max;
    }
    
    virtual ~FixedList() = default;
    
    void push_back(const T& value) {
        if (this->size() == MaxLen) {
           this->pop_front();
        }
        std::list<T>::push_back(value);
    }
    
     void push_back( T&& value) {
        if (this->size() == MaxLen) {
           this->pop_front();
        }
      std::list<T>::push_back( std::move(value) );
    }
    
    T& operator [ ]( int index)
    {
         if( index >= MaxLen  )
         {
             exit(0);
         }
                 
         auto it = std::next(this->begin(), index);
         return  *it;
    }
    
    int MaxLen;
};



//
//template <typename T, int MaxLen, typename Container=std::deque<T>>
//class FixedQueue : public std::queue<T, Container> {
//public:
//    void push(const T& value) {
//        if (this->size() == MaxLen) {
//           this->c.pop_front();
//        }
//        std::queue<T, Container>::push(value);
//    }
//};





#endif
