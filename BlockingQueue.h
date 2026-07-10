#pragma once

#include <deque>
#include <cstddef>
#include <stdexcept>
#include <utility>

#include <type_traits>
#include <condition_variable>
#include <mutex>

#include <optional>

#include <concepts>

template <typename ty>
concept BlockingQueueElement = std::move_constructible<ty> &&
                               std::destructible<ty>;

template <BlockingQueueElement ty, std::size_t uCapacity> requires (uCapacity > 0U)
class BlockingQueue final {
public:
   using BlockingQueueElement_ty = ty;
private:
   // inneren Status
   std::deque<ty>          dqValues { };

   bool                    boClosed { false };

   std::condition_variable cvNotEmpty;
   std::condition_variable cvNotFull;

   // kein Status
   mutable std::mutex      mtxQueue;

   static constexpr std::size_t uCapacityVal { uCapacity };

public:
   BlockingQueue()                         = default;

   BlockingQueue(BlockingQueue const&)     = delete;
   BlockingQueue(BlockingQueue&&) noexcept = delete;
   ~BlockingQueue() = default;

   BlockingQueue& operator = (BlockingQueue const&)      = delete;
   BlockingQueue& operator = (BlockingQueue &&) noexcept = delete;

   // Methode aufrufen mit Push(std::move(element))
   [[nodiscard]] bool Push(ty value) {
      std::unique_lock theLock{ mtxQueue };

      cvNotFull.wait(theLock, [&] {
         return boClosed || dqValues.size() < uCapacityVal;
         });

      if (boClosed) return false;

      dqValues.emplace_back(std::move(value));

      theLock.unlock();
      cvNotEmpty.notify_one();

      return true;
      }



   template <typename... Args> 
      requires std::constructible_from<ty, Args&&...>
   [[nodiscard]] bool Emplace(Args&&... args) {
      std::unique_lock theLock { mtxQueue };

      cvNotFull.wait(theLock, [&] {
         return boClosed || dqValues.size() < uCapacityVal;   
         });

      if (boClosed) return false;

      dqValues.emplace_back(std::forward<Args>(args)...);

      theLock.unlock();
      cvNotEmpty.notify_one();

      return true;
   }


   [[nodiscard]] std::optional<ty> Pop() {
      std::unique_lock theLock{ mtxQueue };

      cvNotEmpty.wait(theLock, [&] {
         return boClosed || !dqValues.empty();
         });

      //if (dqValues.empty()) return std::nullopt;  // kann nicht eintreten, da Bedingung oben
      if (boClosed) return std::nullopt;

      std::optional<ty> retVal { std::move(dqValues.front()) };
      dqValues.pop_front();

      theLock.unlock();
      cvNotFull.notify_one();

      return retVal;
      }

   void Close() {
      {
         std::lock_guard theLock{ mtxQueue };
         if (boClosed) return;
         boClosed = true;
      }
      cvNotEmpty.notify_all();
      cvNotFull.notify_all();
      }

   std::size_t Size() const {
      std::lock_guard theLock{ mtxQueue };
      return dqValues.size();
      }

   std::size_t Capacity() const {
      std::lock_guard theLock{ mtxQueue };
      return uCapacityVal;
      }

   bool IsClose() const {
      std::lock_guard theLock{ mtxQueue };
      return boClosed;
      }

   bool IsEmpty() const {
      std::lock_guard theLock{ mtxQueue };
      return dqValues.empty();
      }

   bool IsFull() const {
      std::lock_guard theLock{ mtxQueue };
      return dqValues.size() >= uCapacityVal;
      }

// -------------------------------------------------------------------------------------

   [[nodiscard]] bool Try_Push(ty value) {
      {
         std::lock_guard theLock{ mtxQueue };

         if (boClosed || dqValues.size() >= uCapacityVal) {
            return false;
            }

         dqValues.emplace_back(std::move(value));

      }
      cvNotEmpty.notify_one();

      return true;
   }


   template <typename... Args>
      requires std::constructible_from<ty, Args&&...>
   [[nodiscard]] bool Try_Emplace(Args&&... args) {
      {
         std::lock_guard theLock{ mtxQueue };

         if (boClosed || dqValues.size() >= uCapacityVal) return false;

         dqValues.emplace_back(std::forward<Args>(args)...);

      }
      cvNotEmpty.notify_one();

      return true;
   }


   [[nodiscard]] std::optional<ty> Try_Pop() {
      {
         std::lock_guard theLock{ mtxQueue };

         if (boClosed || dqValues.empty()) return std::nullopt;

         std::optional<ty> retVal{ std::move(dqValues.front()) };
         dqValues.pop_front();

      }
      cvNotFull.notify_one();

      return retVal;
   }
};

