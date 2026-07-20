// Test.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include <iostream>
#include <vector>
#include <stdexcept>

#include <format>
#include <ranges>
#include <limits>
#include <algorithm>
#include <numeric>    // für accumulate
#include <concepts>
#include <compare>

#include <print>
#include <stdfloat>


template<typename ty>
concept my_integral = std::is_integral_v<ty> && !std::is_same_v<ty, bool>;


template<my_integral ty>
class IntegerWrapper {
public:
   IntegerWrapper() : mValue{} {}

   IntegerWrapper(ty val) : mValue{ val } {}

   template<my_integral U>  requires (!std::is_same_v<U, ty>)
   explicit IntegerWrapper(U val) {
      Check(val);
      /*
      using common_ty = std::common_type_t<ty, U>;

      common_ty const cval = static_cast<common_ty>(val);
      common_ty const cmin = static_cast<common_ty>(std::numeric_limits<ty>::min());
      common_ty const cmax = static_cast<common_ty>(std::numeric_limits<ty>::max());

      if (cval < cmin || cval > cmax) {
         throw std::out_of_range{ "IntegerWrapper: Wert liegt außerhalb des darstellbaren Bereichs" };
         }
      */
      mValue = static_cast<ty>(val);
      }


   IntegerWrapper(IntegerWrapper const&) = default;
   IntegerWrapper(IntegerWrapper&&) noexcept = default;

   IntegerWrapper& operator=(IntegerWrapper const&)  = default;
   auto operator=(IntegerWrapper&&) noexcept -> IntegerWrapper & = default;  // mal mit trailing
   
   IntegerWrapper& operator = (ty val) {
      mValue = val;
      return *this;
      }

   template<my_integral U>  requires (!std::is_same_v<U, ty>)
   IntegerWrapper& operator = (U val) {
      Check(val);
      mValue = static_cast<ty>(val);
      return *this;
      }

   auto operator <=> (IntegerWrapper const&) const = default;

   ty Get() const { return mValue; }


   void Set(ty val) { 
      mValue = val; 
      }

   template<my_integral U>  requires (!std::is_same_v<U, ty>)
   void Set(U val) {
      Check(val);
      mValue = static_cast<ty>(val);
      }

   bool IsEven() const { return mValue % 2 == 0; }
   bool IsOdd() const { return !IsEven(); }

private:
   template<my_integral U>  requires (!std::is_same_v<U, ty>)
   void Check(U val) {
      using common_ty = std::common_type_t<ty, U>;
      common_ty const cval = static_cast<common_ty>(val);
      common_ty const cmin = static_cast<common_ty>(std::numeric_limits<ty>::min());
      common_ty const cmax = static_cast<common_ty>(std::numeric_limits<ty>::max());

      if (cval < cmin || cval > cmax) {
         throw std::out_of_range{ "IntegerWrapper: Wert liegt außerhalb des darstellbaren Bereichs" };
         }
      }


private:
   ty mValue;
};



template<my_integral ty>
constexpr IntegerWrapper<ty> operator + (IntegerWrapper<ty> const& lhs, IntegerWrapper<ty> const& rhs) noexcept {
   return IntegerWrapper<ty>{lhs.Get() + rhs.Get()};
   }


template<my_integral ty>
constexpr IntegerWrapper <ty> operator + (IntegerWrapper<ty> const& lhs, ty rhs) noexcept {
   return IntegerWrapper<ty>{lhs.Get() + rhs};
   }


template<my_integral ty>
constexpr IntegerWrapper<ty>  operator + (ty lhs, IntegerWrapper<ty> const& rhs) noexcept {
   return IntegerWrapper<ty>{lhs + rhs.Get()};
   }


template<my_integral ty>
std::ostream& operator << (std::ostream& rOut, IntegerWrapper<ty> const& aVal) {
   return rOut << aVal.Get();
   }

template<my_integral ty>
constexpr ty GetMax() noexcept {
   return std::numeric_limits<ty>::max();
   }


template<my_integral ty>
struct std::formatter<IntegerWrapper<ty>> {
   std::formatter<ty> fmtInner;

   constexpr auto parse(std::format_parse_context& ctx) {
      return fmtInner.parse(ctx); // delegiere Parsing an Standardformatter von T
      }

   template<typename FormatContext>
   auto format(IntegerWrapper<ty> const& aValue, FormatContext& ctx) const {
      auto out = ctx.out();
      out = std::format_to(out, "(");
      out = fmtInner.format(aValue.Get(), ctx);
      return std::format_to(out, ")");
      }
};



template<std::ranges::input_range Range>
void WriteRange(Range aRange, std::ostream& rOut, char const* szDelimiter = "\n") {
   using ElemType = std::ranges::range_value_t<Range>;
   std::ostream_iterator<ElemType> outIter(rOut, szDelimiter);
   std::ranges::copy(aRange, outIter);
   }

template<std::ranges::input_range range_ty>
auto CalcSum(range_ty&& aRange) {
   using value_type = std::ranges::range_value_t<range_ty>;
   return std::accumulate(std::ranges::begin(aRange), std::ranges::end(aRange), value_type{});
   }

template <my_integral ty>
void OutputAddresses(std::string const& text, std::vector<IntegerWrapper<ty>> const& data) {
   std::cout << text << ": vector @ " << std::addressof(data) << ", 1st element @ " << std::addressof(data[0]) << std::endl;
   }

template <my_integral ty>
std::vector<IntegerWrapper<ty>> GetTestValues() {
   std::vector<IntegerWrapper<ty>> retVals = { 5, 6, 2, 1, 8, 3, 9, 4, 7, 10, 20, 15, 13, 18, 12, 17, 14, 11, 19, 16, 0 };
   OutputAddresses("Optimiert", retVals);
   return retVals;
   }



// nicht vorher, da Makros der Standardlibrary überschrieben werden
// echtes Windows API Problem mit Makros !!!
#ifdef _WIN32
#include <windows.h>
#endif

int main() {

#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);
#endif
   std::cout << "Testprogramm für C++20 Eigenschaften:\n";
   try {

      //std::println(std::cout, "{}", std::float128_t{ 1 });
  
      //std::vector<IntegerWrapper<int>> test;
      //test = GetTestValues<int>();
      auto test = GetTestValues<int>();
      OutputAddresses("Optimiert", test);

      std::println("{} Testdaten im Programm ", test.size());
 
      auto r = test | std::views::filter([](auto i) { return i.IsEven(); });
      
      std::cout << std::format("\nAusgabe gefilteter Range mit template:\n");
      WriteRange(r, std::cout);
      std::cout << std::format("\nSumme: {:5}\n", CalcSum(r));
      
      std::cout << std::format("\nAusgabe mit for_each und format:\n");
      std::ranges::for_each(r, [](auto i) { std::cout << std::format("{}\n", i); });
      
      std::cout << std::format("\nSortieren und Ausgabe über WriteRange:\n");
      std::ranges::sort(test, std::ranges::less());
      WriteRange(test, std::cout, ", ");

      IntegerWrapper<int> test_overflow(GetMax<long long>());
      std::cout << test_overflow << '\n';
      }
   catch(std::exception& ex) {
      std::cout << std::format("\nexception: {}", ex.what());
      }
   }


