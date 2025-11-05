///                                                                           
/// literal_t                                                                 
/// Copyright (c) 2025 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: MIT                                              
///                                                                           
#pragma once
#include <array>
#include <string_view>
#include <bit>

/// You decide whether literal types throw or not                             
#ifdef LANGULUS_OPTION_SAFE_MODE
   #include <stdexcept>
   #define lgls_has_assumptions
   #define lgls_if_safe(a) a
   #define lgls_if_unsafe(a)
   #define lgls_assume(CONDITION, MESSAGE) \
      if (not static_cast<bool>(CONDITION)) \
         throw ::std::runtime_error {MESSAGE};

   #if defined(_MSC_VER) and not defined(__clang__)
      #define lgls_assume_and_optimize(CONDITION, MESSAGE) \
         if (not static_cast<bool>(CONDITION)) \
            throw ::std::runtime_error {MESSAGE};
   #else
      #define lgls_assume_and_optimize(CONDITION, MESSAGE) \
         if (not static_cast<bool>(CONDITION)) \
            throw ::std::runtime_error {MESSAGE}; \
         [[assume(CONDITION)]]
   #endif
#else
   #define lgls_has_assumptions noexcept
   #define lgls_if_safe(a)
   #define lgls_if_unsafe(a) a
   #define lgls_assume(CONDITION, MESSAGE)
   #define lgls_assume_and_optimize(CONDITION, MESSAGE)
#endif

#if defined(_MSC_VER) and not defined(__clang__)
   #define lgls_inline __forceinline
   #define lgls_pure
#else
   #define lgls_inline __attribute__((always_inline)) inline
   #define lgls_pure __attribute__((pure))
#endif


namespace Langulus
{
   /// Used as a return type in unsupported functions                         
   struct Unsupported {};

   namespace CT
   {
      /// Check if all T are complete (defined), by exploiting sizeof.        
      /// Usefulness of this is limited to the first instantiation, and       
      /// that is how it is used upon reflection. Thankfully, most modern     
      /// compilers do detect if a definition changes between completeness    
      /// checks, so it is unlikely to cause any real harm:                   
      /// https://stackoverflow.com/questions/21119281                        
      template<class...T>
      concept Complete = (sizeof...(T) > 0) and ((sizeof(T) == sizeof(T)) and ...);

      namespace Inner
      {
         template<class...T>
         consteval bool ValidateInner() {
            static_assert(sizeof...(T) > 0,
               "No arguments provided");
            static_assert(((Complete<T> or ::std::is_void_v<T>) and ...),
               "Incomplete type in CT check");
            return true;
         }

         template<class...T>
         consteval bool PartialValidateInner() {
            static_assert(sizeof...(T) > 0, "No arguments provided");
            return true;
         }
      }

      /// Makes sure an error is reported if a CT concept is tested without   
      /// any arguments, or if any argument is an incomplete type, so that    
      /// failures aren't silent.                                             
      ///   @attention 'void' is not considered incomplete in this context    
      template<class...T>
      concept Validate = Inner::ValidateInner<T...>();

      /// Check if all T are literal_t types                                  
      template<class...T>
      concept Literal = Validate<T...> and (T::CTTI_Literal and ...);
      
      /// Supported character types used by LiteralString                     
      template<class...T>
      concept LiteralChar = Validate<T...> and ((
              ::std::same_as<::std::remove_cv_t<T>, char>
           or ::std::same_as<::std::remove_cv_t<T>, wchar_t>
           or ::std::same_as<::std::remove_cv_t<T>, char8_t>
           or ::std::same_as<::std::remove_cv_t<T>, char16_t>
           or ::std::same_as<::std::remove_cv_t<T>, char32_t>
         ) and ...);
      
      /// Check if all T are literal_t strings                                
      template<class...T>
      concept LiteralString = Literal<T...>
          and ((T::ArraySize > 0 and LiteralChar<typename T::value_type>) and ...);
      
      /// Check if all T are literal_t values                                 
      template<class...T>
      concept LiteralValue = Literal<T...> and ((T::ArraySize == 0
          and not ::std::same_as<::std::remove_cv_t<typename T::value_type>, Unsupported>) and ...);
      
      /// Check if all T are literal_t values, but undefined                  
      template<class...T>
      concept LiteralUndefined = Literal<T...>
          and (::std::same_as<::std::remove_cv_t<typename T::value_type>, Unsupported> and ...);
   }

   using Token = ::std::string_view;


   ///                                                                        
   /// Acts as both a single value, or string literal. You can use it as a    
   /// template parameter. The string implementation should be introduced in  
   /// C++26 as std::fixed_string, supposedly...                              
   ///                                                                        
   /// String literals of different sizes result in unique types, and thus    
   /// can't be used in `?:` statements, so I've taken the liberty to allow   
   /// for strings of the form `? "\0\0\0" : "alt"` - left literal has        
   /// `literal_t::ArraySize == 3`, but `size() == 0`. This also allows us to 
   /// do some neat compile speed/memory optimizations by minimizing template 
   /// instantiation via CTAD.                                                
   ///                                                                        
   template<class T, size_t N>
   struct literal_t {
      static_assert(N == 0 or ::std::has_single_bit(N),
         "Modify N to minimize the number of templates");
      static constexpr bool   CTTI_Literal = true;
      static constexpr bool   Undefined = ::std::same_as<T, Unsupported>;
      static constexpr size_t ArraySize = N;

      using storage_type = ::std::array<T, N + 1>;
      storage_type _data {};

      using value_type = T;
      using pointer = value_type*;
      using const_pointer = const value_type*;
      using reference = value_type&;
      using const_reference = const value_type&;
      using iterator = typename storage_type::iterator;
      using const_iterator = typename storage_type::const_iterator;
      using reverse_iterator = typename storage_type::reverse_iterator;
      using const_reverse_iterator = typename storage_type::const_reverse_iterator;
      using difference_type = ptrdiff_t;
      using view_type = ::std::basic_string_view<value_type>;

      static constexpr size_t npos = view_type::npos;

      constexpr literal_t() noexcept = default;

      constexpr literal_t(const value_type& c) noexcept {
         _data[0] = c;
      }

      template<size_t M> requires (M <= N)
      constexpr literal_t(const literal_t<char, M>& other) noexcept {
         for (size_t i = 0; i < M; i++)
            _data[i] = other._data[i];
         _data[M] = 0;
      }

      template<size_t M> requires (M <= N + 1)
      constexpr literal_t(const value_type(&array)[M]) noexcept {
         for (size_t i = 0; i < M; i++)
            _data[i] = array[i];
      }

      constexpr literal_t& operator = (const value_type(&array)[N]) noexcept {
         for (size_t i = 0; i < N; i++)
            _data[i] = array[i];
         return *this;
      }

      ///                                                                     
      /// Iteration                                                           
      ///                                                                     
      constexpr auto begin(this auto&& self) noexcept {
         return self._data.begin();
      }

      constexpr auto end(this auto&& self) noexcept {
         return self._data.begin() + self.size();
      }

      constexpr auto cbegin() const noexcept {
         return _data.cbegin();
      }

      constexpr auto cend(this auto&& self) noexcept {
         return self._data.cbegin() + self.size();
      }

      constexpr auto rbegin(this auto&& self) noexcept {
         return self._data.rbegin() + (N - self.size());
      }

      constexpr auto rend(this auto&& self) noexcept {
         return self._data.rend();
      }

      constexpr auto crbegin(this auto&& self) noexcept {
         return self._data.crbegin() + (N - self.size());
      }

      constexpr auto crend() const noexcept {
         return _data.crend();
      }

      ///                                                                     
      /// Encapsulation                                                       
      ///                                                                     
      constexpr size_t size() const noexcept {
         if constexpr (N > 0 and not Undefined) {
            // This is a slow implementation, but Literals are mostly   
            // used at compile-time, so it shouldn't be an issue        
            auto ptr = _data.data();
            const auto ptrEnd = ptr + N;
            while(ptr != ptrEnd and *ptr)
               ++ptr;
            return ptr - _data.data();
         }
         else return 0;
      }
      
      constexpr bool empty() const noexcept {
         if constexpr (N > 0 and not Undefined)
            return not N or not _data[0];
         else
            return true;
      }
      
      constexpr explicit operator bool () const noexcept {
         if constexpr (Undefined) return false;
         else return _data[0];
      }

      ///                                                                     
      /// Access                                                              
      /// @attention 'n' is always 0 when N == 0                              
      constexpr decltype(auto) operator [] (this auto&& self, [[maybe_unused]] size_t n)
      lgls_has_assumptions {
         if constexpr (N > 0) {
            #ifdef LANGULUS_OPTION_SAFE_MODE
               //if not consteval {
                  if (n >= self.size())
                     throw ::std::range_error("subscript index outside literal_t limits");
               //}
            #endif
            return self._data[n];
         }
         else return self._data[0];
      }

      constexpr decltype(auto) at(this auto&& self, size_t n) {
         return self._data.at(n);
      }

      constexpr decltype(auto) front(this auto&& self) noexcept {
         return self._data.front();
      }

      constexpr decltype(auto) back(this auto&& self) noexcept {
         return self._data[self.size() - 1];
      }

      constexpr auto data(this auto&& self) noexcept {
         return self._data.data();
      }

      constexpr auto c_str() const noexcept {
         return _data.data();
      }

      ///                                                                     
      /// Retype                                                              
      ///                                                                     
      /// Get a resized Literal with the same properties                      
      template<size_t M>
      using Resized = literal_t<value_type, ::std::bit_ceil(M)>;

   protected:
      template<class, size_t>
      friend struct literal_t;

      template<size_t pos, size_t count, size_t size>
      consteval static size_t clamp() {
         if constexpr (pos >= size)
            return 0;
         return count < size - pos ? count : size - pos;
      }

      constexpr view_type sv() const { return *this; }

   public:
      /// Implicit cast to a first value, if N == 0                           
      constexpr operator T() const noexcept requires (N == 0) {
         return _data[0];
      }
      
      /// Implicit cast to a string view, if N > 0                            
      constexpr operator view_type() const noexcept requires (N > 0) {
         return {data(), size()};
      }

      /// Get a region of the string                                          
      constexpr literal_t substr(size_t pos = 0, size_t count = npos) const noexcept {
         literal_t result;
         const size_t s = size();
         if (pos >= s)
            return result;
         
         if (count > s - pos)
            count = s - pos;         
         
         for (size_t i = 0; i < count; ++i)
            result._data[i] = _data[pos + i];
         result._data[count] = 0;
         return result;
      }

      /// Find                                                                
      template <size_t M>
      constexpr size_t find(const Resized<M>& str, size_t pos = 0) const noexcept {
         if constexpr (M > N)
            return npos;
         return sv().find(str.sv(), pos);
      }
      constexpr size_t find(const view_type& view, size_t pos = 0) const noexcept {
         return sv().find(view, pos);
      }
      constexpr size_t find(const value_type* s, size_t pos, size_t n) const {
         return sv().find(s, pos, n);
      }
      constexpr size_t find(const value_type* s, size_t pos = 0) const {
         return sv().find(s, pos);
      }
      constexpr size_t find(value_type c, size_t pos = 0) const noexcept {
         return sv().find(c, pos);
      }

      /// Find in reverse                                                     
      template <size_t M>
      constexpr size_t rfind(const Resized<M>& str, size_t pos = npos) const noexcept {
         if constexpr (M > N)
            return npos;
         return sv().rfind(str.sv(), pos);
      }
      constexpr size_t rfind(const view_type& view, size_t pos = npos) const noexcept {
         return sv().rfind(view, pos);
      }
      constexpr size_t rfind(const value_type* s, size_t pos, size_t n) const {
         return sv().rfind(s, pos, n);
      }
      constexpr size_t rfind(const value_type* s, size_t pos = npos) const {
         return sv().rfind(s, pos);
      }
      constexpr size_t rfind(value_type c, size_t pos = npos) const noexcept {
         return sv().rfind(c, pos);
      }

      /// Find the first of                                                   
      template <size_t M>
      constexpr size_t find_first_of(const Resized<M>& str, size_t pos = 0) const noexcept {
         if constexpr (M > N)
            return npos;
         return sv().find_first_of(str.sv(), pos);
      }
      constexpr size_t find_first_of(const view_type& view, size_t pos = 0) const noexcept {
         return sv().find_first_of(view, pos);
      }
      constexpr size_t find_first_of(const value_type* s, size_t pos, size_t n) const {
         return sv().find_first_of(s, pos, n);
      }
      constexpr size_t find_first_of(const value_type* s, size_t pos = 0) const {
         return sv().find_first_of(s, pos);
      }
      constexpr size_t find_first_of(value_type c, size_t pos = 0) const noexcept {
         return sv().find_first_of(c, pos);
      }

      /// Find the last of                                                    
      template <size_t M>
      constexpr size_t find_last_of(const Resized<M>& str, size_t pos = npos) const noexcept {
         if constexpr (M > N)
            return npos;
         return sv().find_last_of(str.sv(), pos);
      }
      constexpr size_t find_last_of(const view_type& view, size_t pos = npos) const noexcept {
         return sv().find_last_of(view, pos);
      }
      constexpr size_t find_last_of(const value_type* s, size_t pos, size_t n) const {
         return sv().find_last_of(s, pos, n);
      }
      constexpr size_t find_last_of(const value_type* s, size_t pos = npos) const {
         return sv().find_last_of(s, pos);
      }
      constexpr size_t find_last_of(value_type c, size_t pos = npos) const noexcept {
         return sv().find_last_of(c, pos);
      }

      /// Find the first NOT of                                               
      template <size_t M>
      constexpr size_t find_first_not_of(const Resized<M>& str, size_t pos = 0) const noexcept {
         if constexpr (M > N)
            return npos;
         return sv().find_first_not_of(str.sv(), pos);
      }
      constexpr size_t find_first_not_of(const view_type& view, size_t pos = 0) const noexcept {
         return sv().find_first_not_of(view, pos);
      }
      constexpr size_t find_first_not_of(const value_type* s, size_t pos, size_t n) const {
         return sv().find_first_not_of(s, pos, n);
      }
      constexpr size_t find_first_not_of(const value_type* s, size_t pos = 0) const {
         return sv().find_first_not_of(s, pos);
      }
      constexpr size_t find_first_not_of(value_type c, size_t pos = 0) const noexcept {
         return sv().find_first_not_of(c, pos);
      }

      /// Find the last NOT of                                                
      template <size_t M>
      constexpr size_t find_last_not_of(const Resized<M>& str, size_t pos = npos) const noexcept {
         if constexpr (M > N)
            return npos;
         return sv().find_last_not_of(str.sv(), pos);
      }
      constexpr size_t find_last_not_of(const view_type& view, size_t pos = npos) const noexcept {
         return sv().find_last_not_of(view, pos);
      }
      constexpr size_t find_last_not_of(const value_type* s, size_t pos, size_t n) const {
         return sv().find_last_not_of(s, pos, n);
      }
      constexpr size_t find_last_not_of(const value_type* s, size_t pos = npos) const {
         return sv().find_last_not_of(s, pos);
      }
      constexpr size_t find_last_not_of(value_type c, size_t pos = npos) const noexcept {
         return sv().find_last_not_of(c, pos);
      }

      /// Compare                                                             
      constexpr int compare(view_type v) const noexcept {
         return sv().compare(v);
      }
      constexpr int compare(size_t pos1, size_t count1, view_type v) const {
         return sv().compare(pos1, count1, v);
      }
      constexpr int compare(size_t pos1, size_t count1, view_type v, size_t pos2, size_t count2) const {
         return sv().compare(pos1, count1, v, pos2, count2);
      }
      constexpr int compare(const value_type* s) const {
         return sv().compare(s);
      }
      constexpr int compare(size_t pos1, size_t count1, const value_type* s) const {
         return sv().compare(pos1, count1, s);
      }
      constexpr int compare(size_t pos1, size_t count1, const value_type* s, size_t count2) const {
         return sv().compare(pos1, count1, s, count2);
      }

      /// Starts with                                                         
      constexpr bool starts_with(view_type v) const noexcept {
         return sv().substr(0, v.size()) == v;
      }
      constexpr bool starts_with(char c) const noexcept {
         return not empty() and ::std::char_traits<T>::eq(front(), c);
      }
      constexpr bool starts_with(const value_type* s) const noexcept {
         return starts_with(view_type(s));
      }

      /// Ends with                                                           
      constexpr bool ends_with(view_type sv) const noexcept {
         return size() >= sv.size() && compare(size() - sv.size(), npos, sv) == 0;
      }
      constexpr bool ends_with(value_type c) const noexcept {
         return !empty() && ::std::char_traits<T>::eq(back(), c);
      }
      constexpr bool ends_with(const value_type* s) const {
         return ends_with(view_type(s));
      }

      /// Contains                                                            
      constexpr bool contains(view_type sv) const noexcept {
         return find(sv) != npos;
      }
      constexpr bool contains(value_type c) const noexcept {
         return find(c) != npos;
      }
      constexpr bool contains(const value_type* s) const {
         return find(s) != npos;
      }

      void swap(literal_t& other) noexcept(std::is_nothrow_swappable_v<storage_type>) {
         _data.swap(other._data);
      }

      /// Append a string literal                                             
      ///   @attention will never allocate a bigger literal                   
      constexpr literal_t& operator += (const CT::LiteralString auto& rhs) noexcept {
         auto d = data() + size();
         auto s = rhs.data();
         const auto sEnd = rhs.data() + rhs.size() + 1;
         while (d != data() + ArraySize and s != sEnd)
            *(d++) = *(s++); 
         return *this;
      }

      template<CT::LiteralChar C, size_t M>
      constexpr literal_t& operator += (const C(&rhs)[M]) noexcept {
         auto d = data() + size();
         auto s = rhs;
         const auto sEnd = rhs + M;
         while (d != data() + ArraySize and s != sEnd)
            *(d++) = *(s++); 
         return *this;
      }
   };

   /// CTAD                                                                   
   literal_t() ->literal_t<Unsupported, 0>;

   template<class T>
   literal_t(const T&) -> literal_t<T, 0>;
   
   /// CTAD witch a cheeky build optimization                                 
   template<class T, size_t N>
   literal_t(const T(&)[N]) -> literal_t<T, ::std::bit_ceil(N)>;


   ///                                                                        
   /// Literal == Literal                                                     
   template<CT::Literal LHS, CT::Literal RHS>
   constexpr bool operator == (const LHS& lhs, const RHS& rhs) {
      if constexpr (CT::LiteralString<LHS, RHS>) {
         // Both are strings                                            
         if (lhs.size() != rhs.size())
            return false;
      
         for (size_t i = 0; i < lhs.size(); ++i) {
            if (lhs[i] != rhs[i])
               return false;
         }
         return true;
      }
      else if constexpr (CT::LiteralString<LHS>) {
         // LHS is string, RHS is value/undefined                       
         if constexpr (CT::LiteralUndefined<RHS>)
            return lhs.empty();
         else if constexpr (::std::equality_comparable_with<typename LHS::value_type, typename RHS::value_type>)
            return (lhs.empty() and rhs.empty()) or (lhs.size() == 1 and lhs[0] == rhs[0]);
         else
            return false;
      }
      else if constexpr (CT::LiteralString<RHS>) {
         // LHS is value/undefined, RHS is string                       
         if constexpr (CT::LiteralUndefined<LHS>)
            return rhs.empty();
         else if constexpr (::std::equality_comparable_with<typename LHS::value_type, typename RHS::value_type>)
            return (lhs.empty() and rhs.empty()) or (rhs.size() == 1 and lhs[0] == rhs[0]);
         else
            return false;
      }
      else if constexpr (::std::equality_comparable_with<typename LHS::value_type, typename RHS::value_type>) {
         // Both are values/undefined and comparable                    
         return lhs[0] == rhs[0];
      }
      else {
         // Both are values/undefined and uncomparable, and can be the  
         // same only if both are undefined                             
         return CT::LiteralUndefined<LHS, RHS>;
      }
   }

   /// Literal == View                                                        
   template<CT::LiteralString S>
   constexpr bool operator == (const S& lhs, typename S::view_type rhs) {
      return static_cast<typename S::view_type>(lhs) == rhs;
   }

   /// View == Literal                                                        
   template<CT::LiteralString S>
   constexpr bool operator == (typename S::view_type lhs, const S& rhs) {
      return static_cast<typename S::view_type>(rhs) == lhs;
   }

   /// View == Undefined                                                      
   template<CT::LiteralUndefined S>
   constexpr bool operator == (const ::std::string_view& lhs, const S&) {
      return lhs.empty();
   }

   /// Literal == Array                                                       
   template<CT::LiteralString S, size_t N>
   constexpr bool operator == (const S& lhs, const typename S::value_type(&rhs)[N]) {
      return static_cast<typename S::view_type>(rhs) == lhs;
   }

   /// Array == Literal                                                       
   template<CT::LiteralString S, size_t N>
   constexpr bool operator == (const typename S::value_type(&lhs)[N], const S& rhs) {
      return static_cast<typename S::view_type>(lhs) == rhs;
   }

   /// LiteralValue == Array                                                  
   template<CT::LiteralValue S, size_t N>
   constexpr bool operator == (const S& lhs, const typename S::value_type(&rhs)[N]) {
      return lhs[0] == rhs[0];
   }

   /// Array == LiteralValue                                                  
   template<CT::LiteralValue S, size_t N>
   constexpr bool operator == (const typename S::value_type(&lhs)[N], const S& rhs) {
      return lhs[0] == rhs[0];
   }

   /// LiteralUndefined == Array                                              
   template<CT::LiteralUndefined S, CT::LiteralChar C, size_t N>
   constexpr bool operator == (const S&, const C(&rhs)[N]) {
      return rhs[0] == '\0';
   }

   /// Array == LiteralUndefined                                              
   template<CT::LiteralUndefined S, CT::LiteralChar C, size_t N>
   constexpr bool operator == (const C(&lhs)[N], const S&) {
      return lhs[0] == '\0';
   }


   ///                                                                        
   /// Literal <=> Literal                                                    
   constexpr auto operator <=> (
      const CT::LiteralString auto& lhs,
      const CT::LiteralString auto& rhs
   ) {
      using lhs_type = std::decay_t<decltype(lhs)>;
      using sv_type = typename lhs_type::view_type;
      return static_cast<sv_type>(lhs) <=> rhs;
   }

   /// Literal <=> View                                                       
   template<CT::LiteralString S>
   constexpr auto operator <=> (const S& lhs, const typename S::view_type& rhs) {
      return static_cast<typename S::view_type>(lhs) <=> rhs;
   }
   
   /// View <=> Literal                                                       
   template<CT::LiteralString S>
   constexpr auto operator <=> (const typename S::view_type& lhs, const S& rhs) {
      return lhs <=> static_cast<typename S::view_type>(rhs);
   }
   
   /// Literal <=> Array                                                      
   template<CT::LiteralString S, size_t N>
   constexpr auto operator <=> (const S& lhs, const typename S::value_type(&rhs)[N]) {
      using sv_type = typename S::view_type;
      return static_cast<sv_type>(lhs) <=> sv_type {rhs};
   }
   
   /// Array <=> Literal                                                      
   template<CT::LiteralString S, size_t N>
   constexpr auto operator <=> (const typename S::value_type(&lhs)[N], const S& rhs) {
      using sv_type = typename S::view_type;
      return sv_type {lhs} <=> static_cast<sv_type>(rhs);
   }
   

   ///                                                                        
   /// Concatenation                                                          
   ///                                                                        
   template<CT::LiteralString LHS, CT::LiteralString RHS>
   constexpr auto operator + (const LHS& lhs, const RHS& rhs) {
      typename LHS::template Resized<LHS::ArraySize + RHS::ArraySize> result {lhs};
      result += rhs;
      return result;
   }

   template<CT::LiteralChar C, size_t N, CT::LiteralString RHS>
   constexpr auto operator + (const C(&lhs)[N], const RHS& rhs) {
      typename RHS::template Resized<N + RHS::ArraySize> result {lhs};
      result += rhs;
      return result;
   }

   template<CT::LiteralChar C, size_t N, CT::LiteralString LHS>
   constexpr auto operator + (const LHS& lhs, const C(&rhs)[N]) {
      typename LHS::template Resized<LHS::ArraySize + N> result {lhs};
      result += rhs;
      return result;
   }

   template<CT::LiteralChar C, CT::LiteralString RHS>
   constexpr auto operator + (C lhs, const RHS& rhs) {
      typename RHS::template Resized<1 + RHS::ArraySize> result {lhs};
      result += rhs;
      return result;
   }

   template<CT::LiteralChar C, CT::LiteralString LHS>
   constexpr auto operator + (const LHS& lhs, C rhs) {
      typename LHS::template Resized<1 + LHS::ArraySize> result {lhs};
      result += rhs;
      return result;
   }
}

namespace std
{
   /// Swap two strings                                                       
   template<::Langulus::CT::LiteralString S>
   void swap(S& lhs, S& rhs) noexcept(noexcept(lhs.swap(rhs))) {
      lhs.swap(rhs);
   }

   ///                                                                        
   /// Hash support                                                           
   ///                                                                        
   template<class C, size_t N>
   struct hash<::Langulus::literal_t<C, N>> {
      using argument_type = ::Langulus::literal_t<C, N>;

      lgls_inline
      size_t operator()(const argument_type& str) const {
         using sv_t = typename argument_type::string_view_type;
         return hash<sv_t>()(static_cast<sv_t>(str));
      }
   };
}
