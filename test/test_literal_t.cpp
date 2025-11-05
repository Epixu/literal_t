///                                                                           
/// literal_t                                                                 
/// Copyright (c) 2025 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: MIT                                              
///                                                                           
#include <catch2/catch.hpp>
#include <Langulus/Literal.hpp>
#include <string>

using namespace Langulus;

namespace
{
   constexpr literal_t emptyUndefined{};
   constexpr literal_t emptyString2 = "";
   constexpr literal_t emptyString3 = "\0";
   constexpr literal_t emptyString4 = "\0\0\0";
   constexpr literal_t fixedString = "Test String";
   constexpr const char carrayString[] = "Test String";
   constexpr const char* cptrString = "Test String";
   constexpr ::std::string_view viewString = "Test String";
   ::std::string justString = "Test String";

   constexpr literal_t fixedValue = 5.5f;
   constexpr literal_t fixedValueChar = 'a';

   template<literal_t SENT_AS_TEMPLATE_ARGUMENT>
   consteval auto LiteralAsTemplateArgument() {
      return SENT_AS_TEMPLATE_ARGUMENT;
   }
}


///                                                                           
/// CT::Literal                                                               
///                                                                           
SCENARIO("Testing CT::Literal", "[ct]") {
   //static_assert(CT::Literal<>); // shouldn't compile
   static_assert(    CT::Literal<decltype(emptyUndefined)>);
   static_assert(    CT::Literal<decltype(fixedString)>);
   static_assert(    CT::Literal<decltype(fixedValue)>);
   static_assert(    CT::Literal<decltype(fixedValueChar)>);
   static_assert(not CT::Literal<decltype(justString)>);
   static_assert(not CT::Literal<decltype(carrayString)>);
   static_assert(not CT::Literal<decltype(viewString)>);

   static_assert(    CT::Literal<decltype(fixedString), decltype(emptyUndefined), decltype(fixedValue)>);
   static_assert(not CT::Literal<decltype(fixedString), decltype(emptyUndefined), decltype(justString)>);
}


///                                                                           
/// CT::LiteralUndefined                                                      
///                                                                           
SCENARIO("Testing CT::LiteralUndefined", "[ct]") {
   //static_assert(CT::LiteralUndefined<>); // shouldn't compile
   static_assert(    CT::LiteralUndefined<decltype(emptyUndefined)>);
   static_assert(not CT::LiteralUndefined<decltype(emptyString2)>);
   static_assert(not CT::LiteralUndefined<decltype(emptyString3)>);
   static_assert(not CT::LiteralUndefined<decltype(emptyString4)>);
   static_assert(not CT::LiteralUndefined<decltype(fixedString)>);
   static_assert(not CT::LiteralUndefined<decltype(justString)>);
   static_assert(not CT::LiteralUndefined<decltype(carrayString)>);
   static_assert(not CT::LiteralUndefined<decltype(viewString)>);
   static_assert(not CT::LiteralUndefined<decltype(fixedValue)>);
   static_assert(not CT::LiteralUndefined<decltype(fixedValueChar)>);

   static_assert(    CT::LiteralUndefined<decltype(emptyUndefined), decltype(emptyUndefined)>);
   static_assert(not CT::LiteralUndefined<decltype(emptyUndefined), decltype(emptyString3)>);
}


///                                                                           
/// CT::LiteralString                                                         
///                                                                           
SCENARIO("Testing CT::LiteralString", "[ct]") {
   //static_assert(CT::LiteralString<>); // shouldn't compile
   static_assert(not CT::LiteralString<decltype(emptyUndefined)>);
   static_assert(    CT::LiteralString<decltype(emptyString2)>);
   static_assert(    CT::LiteralString<decltype(emptyString3)>);
   static_assert(    CT::LiteralString<decltype(emptyString4)>);
   static_assert(    CT::LiteralString<decltype(fixedString)>);
   static_assert(not CT::LiteralString<decltype(justString)>);
   static_assert(not CT::LiteralString<decltype(carrayString)>);
   static_assert(not CT::LiteralString<decltype(viewString)>);
   static_assert(not CT::LiteralString<decltype(fixedValue)>);
   static_assert(not CT::LiteralString<decltype(fixedValueChar)>);

   static_assert(    CT::LiteralString<decltype(fixedString), decltype(emptyString3), decltype(emptyString4)>);
   static_assert(not CT::LiteralString<decltype(fixedString), decltype(emptyString3), decltype(justString)>);
}


///                                                                           
/// CT::LiteralValue                                                          
///                                                                           
SCENARIO("Testing CT::LiteralValue", "[ct]") {
   //static_assert(CT::LiteralValue<>); // shouldn't compile
   static_assert(not CT::LiteralValue<decltype(emptyUndefined)>);
   static_assert(not CT::LiteralValue<decltype(emptyString2)>);
   static_assert(not CT::LiteralValue<decltype(emptyString3)>);
   static_assert(not CT::LiteralValue<decltype(emptyString4)>);
   static_assert(not CT::LiteralValue<decltype(fixedString)>);
   static_assert(not CT::LiteralValue<decltype(justString)>);
   static_assert(not CT::LiteralValue<decltype(carrayString)>);
   static_assert(not CT::LiteralValue<decltype(viewString)>);
   static_assert(    CT::LiteralValue<decltype(fixedValue)>);
   static_assert(    CT::LiteralValue<decltype(fixedValueChar)>);
                     
   static_assert(    CT::LiteralValue<decltype(fixedValue), decltype(fixedValueChar)>);
   static_assert(not CT::LiteralValue<decltype(fixedValue), decltype(fixedString)>);
}


///                                                                           
/// CT::LiteralChar                                                           
///                                                                           
SCENARIO("Testing CT::LiteralChar", "[ct]") {
   //static_assert(CT::LiteralChar<>); // shouldn't compile
   static_assert(    CT::LiteralChar<char, wchar_t, char8_t, char16_t, char32_t>);
   static_assert(not CT::LiteralChar<char, wchar_t, char8_t, char16_t, int>);
}


///                                                                           
/// Literal strings                                                           
///                                                                           
TEMPLATE_TEST_CASE("Testing literal strings", "[ct]",
   char, wchar_t, char8_t, char16_t, char32_t
) {
   STATIC_REQUIRE(    LiteralAsTemplateArgument<"string">());
   STATIC_REQUIRE(    LiteralAsTemplateArgument<5.5f>());
   STATIC_REQUIRE(not LiteralAsTemplateArgument<"">());
   STATIC_REQUIRE(not LiteralAsTemplateArgument<0>());

   WHEN("Constructed") {
      literal_t defaultConstructed;
      REQUIRE(not defaultConstructed);
      REQUIRE(    defaultConstructed.size() == 0);
      REQUIRE(    defaultConstructed.empty() == true);
      REQUIRE(    defaultConstructed == emptyUndefined);
      REQUIRE(    defaultConstructed == emptyString2);
      REQUIRE(    defaultConstructed == emptyString3);
      REQUIRE(    defaultConstructed == emptyString4);

      constexpr literal_t defaultConstructedCxpr;
      STATIC_REQUIRE(not defaultConstructedCxpr);
      STATIC_REQUIRE(    defaultConstructedCxpr.size() == 0);
      STATIC_REQUIRE(    defaultConstructedCxpr.empty() == true);
      STATIC_REQUIRE(    defaultConstructedCxpr == emptyUndefined);
      STATIC_REQUIRE(    defaultConstructedCxpr == emptyString2);
      STATIC_REQUIRE(    defaultConstructedCxpr == emptyString3);
      STATIC_REQUIRE(    defaultConstructedCxpr == emptyString4);

      literal_t arrayConstructed = "array constructed";
      REQUIRE(arrayConstructed);
      REQUIRE(arrayConstructed.size() == 17);
      REQUIRE(arrayConstructed.empty() == false);
      REQUIRE(arrayConstructed != emptyUndefined);
      REQUIRE(arrayConstructed != emptyString2);
      REQUIRE(arrayConstructed != emptyString3);
      REQUIRE(arrayConstructed != emptyString4);

      constexpr literal_t arrayConstructedCxpr = "array constructed";
      STATIC_REQUIRE(arrayConstructedCxpr);
      STATIC_REQUIRE(arrayConstructedCxpr.size() == 17);
      STATIC_REQUIRE(arrayConstructedCxpr.empty() == false);
      STATIC_REQUIRE(arrayConstructedCxpr != emptyUndefined);
      STATIC_REQUIRE(arrayConstructedCxpr != emptyString2);
      STATIC_REQUIRE(arrayConstructedCxpr != emptyString3);
      STATIC_REQUIRE(arrayConstructedCxpr != emptyString4);

      literal_t emptyArrayConstructed = "";
      REQUIRE(not emptyArrayConstructed);
      REQUIRE(    emptyArrayConstructed.size() == 0);
      REQUIRE(    emptyArrayConstructed.empty() == true);
      REQUIRE(    emptyArrayConstructed == emptyUndefined);
      REQUIRE(    emptyArrayConstructed == emptyString2);
      REQUIRE(    emptyArrayConstructed == emptyString3);
      REQUIRE(    emptyArrayConstructed == emptyString4);

      constexpr literal_t emptyArrayConstructedCxpr = "";
      STATIC_REQUIRE(not emptyArrayConstructedCxpr);
      STATIC_REQUIRE(    emptyArrayConstructedCxpr.size() == 0);
      STATIC_REQUIRE(    emptyArrayConstructedCxpr.empty() == true);
      STATIC_REQUIRE(    emptyArrayConstructedCxpr == emptyUndefined);
      STATIC_REQUIRE(    emptyArrayConstructedCxpr == emptyString2);
      STATIC_REQUIRE(    emptyArrayConstructedCxpr == emptyString3);
      STATIC_REQUIRE(    emptyArrayConstructedCxpr == emptyString4);
   }

   WHEN("Assigned") {
      literal_t local = fixedString;
      local = carrayString;
      REQUIRE(local == "Test String");
   }

   WHEN("Iterated") {
      for (size_t i = 0; i < fixedString.size(); ++i) {
         REQUIRE(fixedString[i] == carrayString[i]);
      }

      std::string accumulate;
      for (auto& c : fixedString)
         accumulate += c;
      REQUIRE(accumulate == "Test String");
   }

   WHEN("Accessed") {
      lgls_if_safe(volatile size_t idx = fixedString.size() + 1);
      lgls_if_safe(REQUIRE_THROWS(fixedString[idx]));
      STATIC_REQUIRE(fixedString[0] == carrayString[0]);
      //STATIC_REQUIRE(fixedString[fixedString.size() + 1]); // shouldn't compile
   }

   WHEN("Resized") {

   }

   WHEN("Substring") {

   }

   WHEN("Searched") {

   }

   WHEN("Compared") {
      literal_t local = fixedString;
      REQUIRE(local == cptrString);
   }

   WHEN("Swapped") {

   }

   WHEN("Concatenated") {

   }

   WHEN("Hashed") {

   }
}
