#include <concepts>
#include <print>

#include <vector>
#include <any>

#include <variant>


template<typename derived_ty>
class Printable_Base {
public:
   std::string ToString() const {
      return Derived().ToStringImpl();
   }

   void Print() const {
      std::println("Printable {}", ToString());
   }

private:
   derived_ty const& Derived() const {
      return static_cast<derived_ty const&>(*this);
   }
};


template<typename ty>
concept printable_ty = requires(ty const& obj) {
   { obj.ToString() } -> std::convertible_to<std::string>;
   { obj.Print() } -> std::same_as<void>;
};

class User final : public Printable_Base<User> {
private:
   std::string strName;

public:
   explicit User(std::string strNewName) : strName(std::move(strNewName)) {}

   // Hilfsmethode für CRTP
   std::string ToStringImpl() const { return std::format("User: {}", strName); }

   // Methoden für User
   std::string const& Name() const { return strName; }
   void Rename(std::string strNewName) { strName = std::move(strNewName); }
};

template <typename ty>
concept user_ty = requires(ty const& const_obj, ty & obj, std::string str) {
   { const_obj.Name() } -> std::same_as<std::string const&>;
   { obj.Rename(str) } -> std::same_as<void>;
};




class Order final : public Printable_Base<Order> {
private:
   int iId{};

public:
   explicit Order(int iNewId) : iId(iNewId) {}

   std::string ToStringImpl() const { return std::format("Order: {}", iId); }

   int Id() const { return iId; }
   void SetId(int iNewId) { iId = iNewId; }
};

template <typename ty>
concept order_ty = requires (ty const& const_obj, ty & obj, int val) {
   { const_obj.Id() } -> std::same_as<int>;
   { obj.SetId(val) } -> std::same_as<void>;
};




template<printable_ty ty>
void Print(ty const& obj) {
   obj.Print();
}


template<typename>
inline constexpr bool always_false_v = false;

template <typename ty>
void Work(ty& val) {
   if constexpr (user_ty<ty>) {
      std::println("if constexpr user: {}", val.Name());
   }
   else if constexpr (order_ty<ty>) {
      std::println("if constexpr order: {}", val.Id());
   }
   else {
      static_assert(always_false_v<ty>, "Work<ty>: Keines der erwarteten Concepts trifft zu.");
   }
}



static void Process(std::any const& val) {
   if (auto const* pUser = std::any_cast<User>(&val); pUser != nullptr) {
      std::println("from any: {}", pUser->Name());
      }
   else if (auto const* pOrder = std::any_cast<Order>(&val); pOrder != nullptr) {
      std::println("from any order: {}", pOrder->Id());
      }
   else {
      std::println("unerwartetes any Objekt");
      // throw exception
      }
   }

template <typename ty, typename fn_ty>
   requires std::invocable<fn_ty,ty const&>
void ProcessOne(std::any const& val, fn_ty&& fn ) {
   if (auto const* pVal = std::any_cast<ty>(&val); pVal != nullptr) {
      std::invoke(std::forward<fn_ty>(fn), *pVal);
      }
   }




struct printable_visitor {

   template<typename ty>
   void operator()(ty const& obj) const {
      using clean_ty = std::remove_cvref_t<ty>;

      if constexpr (user_ty<clean_ty>) {
         std::print("variant user constexpr: ");
         std::println("Name = {}", obj.Name());
         }
      else if constexpr (order_ty<clean_ty>) {
         std::print("variant order constexpr: ");
         std::println("Id = {}", obj.Id());
         }
      else {
         static_assert(always_false_v<clean_ty>, "printable_visitor: Keines der erwarteten Concepts trifft zu.");
         }
      }

};


struct processing_visitor {

   template<typename ty>
   void operator()(ty& obj) const {
      using clean_ty = std::remove_cvref_t<ty>;

      if constexpr (user_ty<clean_ty>) {
         if (obj.Name() == "Volker") obj.Rename("Volker (!!!)");
         }
      else if constexpr (order_ty<clean_ty>) {
         if (obj.Id() == 47) obj.SetId(4711);
         }
      else {
         static_assert(always_false_v<clean_ty>, "printable_visitor: Keines der erwarteten Concepts trifft zu.");
      }
   }

};



template<typename target_ty, typename fn_ty>
   requires std::invocable<fn_ty, target_ty&>
struct select_type_visitor {
   fn_ty fnCall;

   template<typename ty>
   void operator()(ty& obj) {
      using clean_ty = std::remove_cvref_t<ty>;

      if constexpr (std::same_as<clean_ty, target_ty>) {
         std::invoke(fnCall, obj);
         }
      else {
         // bewusst ignorieren
         }
      }
};

template<typename target_ty, typename fn_ty>
auto MakeSelectTypeVisitor(fn_ty&& fnCall) {
   return select_type_visitor<target_ty, std::remove_cvref_t<fn_ty>> { std::forward<fn_ty>(fnCall) };
   }


struct Test {
   std::string strTest;
};


int main(void) {
   User  aUser { "Volker" };
   Order aOrder { 42 };
   Print(aUser);
   Print(aOrder);

   std::println("-----");
   Work(aUser);
   Work(aOrder);

  // bewusster Fehler für static_assert
  // Test aTest { "Test" };
  // Work(aTest);
  

   {
      std::println("-----");
      // nutzen von std::any
      std::vector<std::any> values;
      values.emplace_back(User  { "Volker" } );
      values.emplace_back(Order { 47 } );
      values.emplace_back(Order { 42 } );
      values.emplace_back(User  { "Bernd" } );
      values.emplace_back(Order { 52 } );

      values.emplace_back(Test  { "Test" });

      for (auto const& val : values) Process(val);

      for (auto const& val : values) ProcessOne<Test>(val, [](Test const& val) { std::println("process one {}", val.strTest); });
   }


   {
      std::println("-----");
      // nutzen von std::variant

      using variant_ty = std::variant<User, Order>;
      std::vector<variant_ty> values;
      values.emplace_back(User  { "Volker" } );
      values.emplace_back(Order { 42 } );
      values.emplace_back(Order { 47 } );
      values.emplace_back(User  { "Bernd" } );
      values.emplace_back(Order { 52 } );

      for (auto const& val : values) std::visit(printable_visitor{}, val);
      for (auto& val : values) std::visit(processing_visitor{}, val);

      for (auto& val : values) {
         std::visit(MakeSelectTypeVisitor<User>([](User& obj) {
               if(obj.Name() == "Bernd") obj.Rename(std::format("{} (xxx)", obj.Name())); }),
            val);
      }

      for (auto const& val : values) std::visit(printable_visitor{}, val);

   }
   return 0;
   }