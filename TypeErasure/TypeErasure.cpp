#include <concepts>
#include <format>
#include <memory>
#include <print>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

template<typename ty>
concept printable_ty =
   requires(ty const& obj) {
      { obj.ToString() } -> std::convertible_to<std::string>;
   };



class AnyPrintable {
private:
   struct Concept {
      virtual ~Concept() = default;

      virtual std::string ToString() const = 0;
      
      virtual std::type_info const& Type() const noexcept = 0;
      virtual void* Ptr() noexcept = 0;
      virtual void const* Ptr() const noexcept = 0;
     // virtual std::unique_ptr<Concept> Clone() const = 0;
      };

   template<printable_ty ty>
   struct Model final : Concept {
      ty theObject;

      explicit Model(ty aObject) : theObject(std::move(aObject)) { }

      std::string ToString() const override { return theObject.ToString();  }

      std::type_info const& Type() const noexcept override { return typeid(ty); }

      void* Ptr() noexcept override { return std::addressof(theObject); }
      void const* Ptr() const noexcept override { return std::addressof(theObject); }

      /*
      std::unique_ptr<Concept> Clone() const override { 
         return std::make_unique<Model<ty>>(theObject);
         }
      */
      };

   std::unique_ptr<Concept> theSelf;
    
public:
   template<typename ty>
      requires printable_ty<std::remove_cvref_t<ty>>
   explicit AnyPrintable(ty&& obj)
      : theSelf(std::make_unique<Model<std::remove_cvref_t<ty>>>(std::forward<ty>(obj))) {
   }

   /*
   AnyPrintable(AnyPrintable const& aOther) : theSelf(aOther.theSelf->Clone()) {  }

   AnyPrintable& operator=(AnyPrintable const& aOther) {
      if (this != std::addressof(aOther)) {
         theSelf = aOther.theSelf->Clone();
         }

      return *this;
      }
   */
   AnyPrintable(AnyPrintable const& aOther) = delete;


   AnyPrintable(AnyPrintable&&) noexcept = default;
   AnyPrintable& operator=(AnyPrintable&&) noexcept = default;

   std::string ToString() const {
      return theSelf->ToString();
      }

   void Print() const {
      std::println("{}", ToString());
      }

   // -------------------

   std::type_info const& Type() const noexcept {
      return theSelf->Type();
      }

   template<typename ty>
   bool Is() const noexcept {
      using stored_ty = std::remove_cvref_t<ty>;
      return Type() == typeid(stored_ty);
      }

   template<typename ty>
   ty& Get() {
      using stored_ty = std::remove_cvref_t<ty>;
      if (!Is<stored_ty>()) {
         throw std::bad_cast{};
         }
      return *static_cast<stored_ty*>(theSelf->Ptr());
      }

   template<typename ty>
   ty const& Get() const {
      using stored_ty = std::remove_cvref_t<ty>;
      if (!Is<stored_ty>()) {
         throw std::bad_cast{};
         }
      return *static_cast<stored_ty const*>(theSelf->Ptr());
      }
};



struct User {
   std::string strName;

   std::string ToString() const {
      return std::format("User: {}", strName);
      }

   void Rename(std::string strNewName) {
      strName = std::move(strNewName);
      }
};

struct Order {
   int iId{};

   std::string ToString() const {
      return std::format("Order: {}", iId);
      }

   void SetId(int iNewId) {
      iId = iNewId;
      }
};

struct Value {
   double flValue{};

   std::string ToString() const {
      return std::format("Order: {}", flValue);
      }

   void SetValue(int flNewVal) {
      flValue = flNewVal;
      }
};




int main() {
   std::vector<AnyPrintable> items;

   items.emplace_back(User { .strName = "Volker" });
   items.emplace_back(Value{ .flValue = 3.14159265359 });
   items.emplace_back(Order{ .iId = 42 });
   items.emplace_back(User{ .strName = "Bernd" });
   items.emplace_back(Order{ .iId = 67 });

   for (AnyPrintable& item : items) item.Print();
   std::println("-----");

   for (AnyPrintable& item : items) {
      if (item.Is<User>()) {
         if(User& user = item.Get<User>(); user.strName == "Volker") user.Rename("Volker (!!!)");
         }
      else if (item.Is<Order>()) {  
         if(Order& order = item.Get<Order>(); order.iId == 42) order.SetId(4711);
         }
      }

   for (AnyPrintable const& aItem : items) {
      aItem.Print();
      }
   }