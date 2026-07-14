#include <concepts>
#include <print>

#include <vector>
#include <any>

#include <variant>
#include <tuple>


class PrintBase {
public:
   virtual std::string ToString() const = 0;

   void Print() const {
      std::println("virtual Print: {}", ToString());
   }
};

class User {
private:
   std::string strName;

public:
   User(std::string strNewName) : strName(std::move(strNewName)) {}

   std::string const& Name() const { return strName; }
   void Rename(std::string strNewName) { strName = std::move(strNewName); }
};

class UserVirt : public User, public PrintBase {
public:
   //using User::User;
   explicit UserVirt(std::string strNewName) : User(std::move(strNewName)) { }

   // virtuelle Methode aus Basis
   std::string ToString() const override { return std::format("virtual: {}", Name()); }
};


// --------------------------------------------------------------------------------


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

class UserCRTP final : public User, public Printable_Base<UserCRTP> {

public:
   explicit UserCRTP(std::string strNewName) : User(std::move(strNewName)) {}

   // Hilfsmethode für CRTP
   std::string ToStringImpl() const { return std::format("UserCRTP: {}", Name()); }
};

template <typename ty>
concept user_ty = requires(ty const& const_obj, ty & obj, std::string str) {
   { const_obj.Name() } -> std::same_as<std::string const&>;
   { obj.Rename(str) } -> std::same_as<void>;
};


class OrderCRTP final : public Printable_Base<OrderCRTP> {
private:
   int iId{};

public:
   explicit OrderCRTP(int iNewId) : iId(iNewId) {}

   std::string ToStringImpl() const { return std::format("OrderCRTP: {}", iId); }

   int Id() const { return iId; }
   void SetId(int iNewId) { iId = iNewId; }
};

template <typename ty>
concept order_ty = requires (ty const& const_obj, ty & obj, int val) {
   { const_obj.Id() } -> std::same_as<int>;
   { obj.SetId(val) } -> std::same_as<void>;
};


class ValueCRTP final : public Printable_Base<ValueCRTP> {
private:
   double flValue;

public:
   explicit ValueCRTP(double newVal) : flValue(std::move(newVal)) {}

   // Hilfsmethode für CRTP
   std::string ToStringImpl() const { return std::format("ValueCRTP: {}", flValue); }

   // Methoden für UserCRTP
   double const& GetValue() const { return flValue; }
   void SetValue(double newVal) { flValue = std::move(newVal); }
};

template <typename ty>
concept value_ty = requires(ty const& const_obj, ty & obj, double val) {
   { const_obj.GetValue() } -> std::same_as<double const&>;
   { obj.SetValue(val) } -> std::same_as<void>;
};


template<printable_ty ty>
void Print(ty const& obj) {
   obj.Print();
}


template<printable_ty ty>
void PrintOverString(ty const& obj) {
   std::println("over: {}", obj.ToString());
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
   else if constexpr (value_ty<ty>) {
      std::println("if constexpr value: {}", val.GetValue());
      }
   else {
      static_assert(always_false_v<ty>, "Work<ty>: Keines der erwarteten Concepts trifft zu.");
   }
}



static void Process(std::any const& val) {
   if (auto const* pUser = std::any_cast<UserCRTP>(&val); pUser != nullptr) {
      std::println("from any: {}", pUser->Name());
      }
   else if (auto const* pOrder = std::any_cast<OrderCRTP>(&val); pOrder != nullptr) {
      std::println("from any order: {}", pOrder->Id());
      }
   else if(auto const* pValue = std::any_cast<ValueCRTP>(&val); pValue != nullptr) {
      std::println("from any value: {}", pValue->GetValue());
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
      else if constexpr (value_ty<clean_ty>) {
         std::print("variant value constexpr: ");
         std::println("ValueCRTP = {}", obj.GetValue());
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
      else if constexpr (value_ty<clean_ty>) {
         if (obj.GetValue() == 3.1415) obj.SetValue(3.14159265359);
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



template <typename... types_ty>
   requires (printable_ty<types_ty> && ...)
void PrintAll(std::tuple<types_ty...> const& tplValues) {
   std::apply([](auto const&... aValues) {
      (std::println("{}", aValues.ToString()), ...);
      }, tplValues);
}


template <printable_ty... types_ty>
std::string ToStringAll(std::tuple<types_ty...> const& tplValues) {
   std::string strResult;
   bool boFirst{ true };
   std::string strSeparator{ ", " };

   std::apply([&](auto const&... values) {
      auto Append = [&](auto const& aValue) {
         if (!boFirst) [[likely]] strResult.append(strSeparator);
         else boFirst = false;
         strResult.append(aValue.ToString());
         };
      (Append(values), ...);
      }, tplValues);

   return strResult;
}


template <printable_ty... types_ty>
class PrintTuple {
public:
   using tuple_ty = std::tuple<types_ty...>;
private:
   tuple_ty data;

public:
   template <typename... Args>
      requires (sizeof...(Args) == sizeof...(types_ty)) &&
               std::constructible_from<tuple_ty, Args...>
   PrintTuple(Args&&... args) : data { std::move(std::forward<Args>(args))...} { }

   std::string ToString() const { return ToStringAll(data); }
   void Print() const { PrintAll(data); }

};



int main(void) {
   UserVirt  vUser { "virtual Name" };
   vUser.Print();

   UserCRTP  aUser { "Volker" };
   OrderCRTP aOrder { 42 };
   ValueCRTP aValue { 3.1415 };
   Print(aUser);
   Print(aOrder);
   Print(aValue);

   std::println("-----");
   Work(aUser);
   Work(aOrder);
   Work(aValue);

  // bewusster Fehler für static_assert
  // Test aTest { "Test" };
  // Work(aTest);
  

   {
      std::println("-----");
      // nutzen von std::any
      std::vector<std::any> values;
      values.emplace_back(UserCRTP  { "Volker" } );
      values.emplace_back(OrderCRTP { 47 } );
      values.emplace_back(OrderCRTP { 42 } );
      values.emplace_back(UserCRTP  { "Bernd" } );
      values.emplace_back(OrderCRTP { 52 } );
      values.emplace_back(ValueCRTP { 3.1415 } );

      values.emplace_back(Test  { "Test" });

      for (auto const& val : values) Process(val);

      for (auto const& val : values) ProcessOne<Test>(val, [](Test const& val) { std::println("process one {}", val.strTest); });
   }


   {
      std::println("-----");
      // nutzen von std::variant

      using variant_ty = std::variant<UserCRTP, OrderCRTP, ValueCRTP>;
      std::vector<variant_ty> values;
      values.emplace_back(UserCRTP  { "Volker" } );
      values.emplace_back(OrderCRTP { 42 } );
      values.emplace_back(OrderCRTP { 47 } );
      values.emplace_back(UserCRTP  { "Bernd" } );
      values.emplace_back(OrderCRTP { 52 } );
      values.emplace_back(ValueCRTP { 3.1415 });

      for (auto const& val : values) std::visit(printable_visitor{}, val);
      for (auto& val : values) std::visit(processing_visitor{}, val);

      for (auto& val : values) {
         std::visit(MakeSelectTypeVisitor<UserCRTP>([](UserCRTP& obj) {
               if(obj.Name() == "Bernd") obj.Rename(std::format("{} (xxx)", obj.Name())); }),
            val);
      }

      for (auto const& val : values) std::visit(printable_visitor{}, val);

   }
   {
      std::println("-----");
      using tuple_ty = std::tuple<UserCRTP, OrderCRTP, ValueCRTP>;
      tuple_ty test{ UserCRTP { "Volker" }, OrderCRTP { 42 }, ValueCRTP { 3.1415 } };
      //PrintAll(test);
      //std::string testStr = ToStringAll(test);
      //std::println("{}", testStr);

      PrintTuple< UserCRTP, OrderCRTP, ValueCRTP> tt { UserCRTP { "Volker" }, OrderCRTP { 42 }, ValueCRTP { 3.1415 } };
      PrintOverString(tt);


   }

   return 0;
   }