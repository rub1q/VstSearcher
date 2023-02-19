#ifndef VstSearcherH
#define VstSearcherH

#include <Classes.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.ComCtrls.hpp>

#include "VirtualTrees.hpp"

#include <unordered_set>
#include <unordered_map>

namespace searcher
{
  constexpr unsigned MIN_SEARCH_REQUEST_LEN = 2;
  constexpr unsigned MAX_SEARCH_REQUEST_LEN = 128;
  constexpr unsigned DEFAULT_INPUT_DELAY 	  = 300; // ms

  template <typename T>
  class ISet
  {
   public:

    virtual __fastcall ~ISet() = default;

    // Overloaded I/O operators to add/delete new elements to container

    virtual ISet& __fastcall operator<<(T&& value); // Add
    virtual ISet& __fastcall operator>>(T&& value); // Delete

    virtual void __fastcall add(T&& value) 	  = 0;
    virtual void __fastcall remove(T&& value) = 0;

    void __fastcall clear();

    bool __fastcall contains(T&& value) const;
    bool __fastcall empty() const;

    const auto& __fastcall getData() const noexcept;

   protected:

    std::unordered_set<T> data;
  };

  // Search options
  enum class SearchOption
  {
    AUTO_EXPAND_NODES,  		        // Automatic expanding nodes where matches are found
    RELEVANT_SORT,     		          // Apply to the entire list sort by relevance
    START_SEARCH_AFTER_BUTTON_CLICK // Start the search after pressing the corresponding button
  };

  class TSearchOptions final : public ISet<SearchOption>
  {
   public:

    TSearchOptions();

    void __fastcall add(SearchOption&& option) override;
    void __fastcall remove(SearchOption&& option) override;
  };

  class TSearchColumns final : public ISet<unsigned>
  {
   public:

    void __fastcall add(unsigned&& iColumn) override;
    void __fastcall remove(unsigned&& iColumn) override;
  };

  struct Matches
  {
    unsigned totalMatches { 0 }; // Number of all matches in a row
    unsigned wordsMatches { 0 }; // Number of matches in the line for the entered words

    Matches() = default;

    explicit Matches(const unsigned a_totalMatches, const unsigned a_wordsMatches)
        : totalMatches(a_totalMatches)
        , wordsMatches(a_wordsMatches)
    {}

    friend bool operator>(const Matches& lhs, const Matches& rhs);
    friend bool operator<(const Matches& lhs, const Matches& rhs);

    Matches& operator+=(const Matches& rhs);
  };

  class ISearcher;

  class DelayTimer final
  {
   public:

    DelayTimer() = default;

    bool __fastcall isActive() const noexcept;

    void __fastcall reset() noexcept;
    void __fastcall start(const unsigned delay, ISearcher* const owner);

   private:

    static void __stdcall TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

   private:

    static inline UINT_PTR   timer_ { 0 };
    static inline ISearcher* owner_ { nullptr };
  };

	class ISearcher
	{
   public:

    TSearchColumns SearchColumns; // Columns that will be searched for
    TSearchOptions SearchOptions; // Search options

   public:

    ISearcher() = default;

    virtual ~ISearcher();

    /// Method of processing the entered search query
    virtual void __fastcall ProcessRequest() = 0;

    /// Method of resetting search results
    virtual void __fastcall ResetSearchResults() = 0;

    /// Method of adding words from the search bar to the container
    ///
    /// @param[in] searchWords - a string with all entered words

    void __fastcall AddWordsToList(const String& searchWords);

    bool __fastcall WordsListEmpty() const noexcept;

    std::size_t __fastcall WordsListSize() const noexcept;

    void __fastcall ClearWordsList() noexcept;

    /// Method for setting the minimum length of a search query
    ///
    /// @param[in] min_len - min. length of the search query

    void __fastcall SetMinRequestLength(const unsigned min_len) noexcept;

    /// Method for setting the maximum length of a search query
    ///
    /// @param[in] max_len - max. length of the search query

    void __fastcall SetMaxRequestLength(const unsigned max_len) noexcept;

    void __fastcall SetRequestLimits(const unsigned min_len, const unsigned max_len) noexcept;

    /// Method for setting the delay value when entering a search query
    ///
    /// @param[in] delay - delay value (in ms)

    void __fastcall SetInputDelay(const unsigned delay) noexcept;

   private:

    unsigned minRequestLen_ { MIN_SEARCH_REQUEST_LEN }; // Minimum search query length (default = MIN_SEARCH_REQUEST_LEN)
    unsigned maxRequestLen_ { MAX_SEARCH_REQUEST_LEN };	// Maximum search query length (default = MAX_SEARCH_REQUEST_LEN)
    unsigned inputDelay_    { DEFAULT_INPUT_DELAY };    // The value of the delay when entering the request (default = DEFAULT_INPUT_DELAY)

    DelayTimer timer_;

    void __fastcall (__closure *TEditDefaultOnChange)(TObject *Sender);
    void __fastcall (__closure *TEditDefaultOnKeyPress)(TObject *Sender, System::WideChar &Key);
    void __fastcall (__closure *TEditDefaultOnKeyUp)(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall (__closure *TEditDefaultOnRightButtonClick)(TObject *Sender);

    void __fastcall edtOnChange(TObject *Sender);
    void __fastcall edtOnKeyPress(TObject *Sender, System::WideChar &Key);
    void __fastcall edtOnKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall edtOnRightButtonClick(TObject *Sender);

   protected:

    std::unordered_set<std::string> words_;  // Container with words

    TButtonedEdit*   edt_;   // Search 'string'
    TLabel*	         lbl_;   // Label 'Total:' (optional)

    bool isInitialized_ { false }; // A flag that indicates that the search has already been initialized

   protected:

    /// Method for initializing objects required for the search operation
    ///
    /// @param[in] Edit  - search 'string'
    /// @param[in] Label - label 'Total:' (optional)

    void __fastcall Init(TButtonedEdit *Edit, TLabel *Label = nullptr);

    /// Method for counting matches in a row
    ///
    /// @param[in] sText - the string in which you want to count the number of matches

    Matches __fastcall CountMatches(const String& sText) const noexcept;

    /// The method of sorting by relevance
    virtual void __fastcall RelevantSort() noexcept = 0;

    /// Method of displaying messages in a popup
    ///
    /// @param[in] msg - message text

    void __fastcall ShowPopupMessage(String&& msg) noexcept;

    /// Method for setting label caption (m_lbl)
    void __fastcall SetLabelCaption(String&& caption) noexcept;

    /// Method for determining the width of the string
    ///
    /// @param[in] handle - canvas handle
    /// @param[in] text   - word, whose width you need to calculate
    /// @return           - string width

    int __fastcall CalculateTextWidth(HANDLE handle, const char* text) const;
	};

  // Searcher for VirtualStringTree (VirtualTreeView)
  class VstSearcher final : public ISearcher
  {
   public:

    VstSearcher() : ISearcher() {};
    explicit VstSearcher(TVirtualStringTree *Tree, TButtonedEdit *Edit, TLabel *Label = nullptr);

    /// Method for initializing objects required for the search operation
    ///
    /// @param[in] Tree  - pointer to VST
    /// @param[in] Edit  - search 'string'
    /// @param[in] Label - label 'Total:' (optional)

    void __fastcall Init(TVirtualStringTree *Tree, TButtonedEdit *Edit, TLabel *Label = nullptr);

    void __fastcall ProcessRequest() override;
    void __fastcall ResetSearchResults() override;

    /// Method of counting matches in the specified node.
    /// Method counts matches in each column passed to SearchColumns
    ///
    /// @param[in] Node - pointer to Node
    /// @return 		    - amount of matches

    Matches __fastcall CountMatchesInNode(TVirtualNode* Node) const;

    /// Method of counting matches in the specified node and column
    ///
    /// @param[in] Node 	  - pointer to Node
    /// @param[in] colIndex - column index
    /// @return 			      - amount of matches

    Matches __fastcall CountMatchesInColumn(TVirtualNode* Node, const int colIndex) const;

    void __fastcall HighlightTreeText(TCanvas* canvas, PVirtualNode Node, TColumnIndex Column, TRect &CellRect) const;

   private:

    int defaultSortColumn_;
    typename Virtualtrees::TSortDirection defaultSortDirection_;

    TVirtualStringTree* vt_;

    std::unordered_map<TVirtualNode*, Matches> matches_;

   private:

    // This class needs for VST->IterateSubTree
    class TVTGetNodeProcRef : public TCppInterfacedObject<TVTGetNodeProc>
    {
     private:

      typedef void __fastcall
      (*TIterateSubtreeCallBack)(TBaseVirtualTree*, PVirtualNode Node, void *Data, bool &Abort);

      TIterateSubtreeCallBack Callback;

     public:

      TVTGetNodeProcRef(TIterateSubtreeCallBack _callback) : Callback(_callback)
      {};

      INTFOBJECT_IMPL_IUNKNOWN(TInterfacedObject);

      void __fastcall Invoke(TBaseVirtualTree* Sender, TVirtualNode *Node, void *Data, bool &Abort) override
      {
        return Callback(Sender, Node, Data, Abort);
      }
    };

    void __fastcall ShowAllRecords() noexcept;
    void __fastcall RelevantSort() noexcept override;

    void __fastcall (__closure *TVTDefaultCompareEvent)(TBaseVirtualTree* Sender,
                                                        PVirtualNode Node1, PVirtualNode Node2,
                                                        TColumnIndex Column, int &Result);
    void __fastcall (__closure *TVTDefaultBeforeCellPaintEvent)(TBaseVirtualTree* Sender,
                                                                Vcl::Graphics::TCanvas* TargetCanvas,
                                                                PVirtualNode Node, TColumnIndex Column,
                                                                TVTCellPaintMode CellPaintMode,
                                                                const System::Types::TRect &CellRect,
                                                                System::Types::TRect &ContentRect);
    void __fastcall (__closure *TVTDefaultHeaderClick)(TVTHeader* Sender, const TVTHeaderHitInfo &HitInfo);

    void __fastcall vstOnCompareNodes(TBaseVirtualTree *Sender, PVirtualNode Node1,
                                      PVirtualNode Node2, TColumnIndex Column, int &Result);
    void __fastcall vstOnBeforeCellPaint(TBaseVirtualTree* Sender,
                                       Vcl::Graphics::TCanvas* TargetCanvas,
                                       PVirtualNode Node, TColumnIndex Column,
                                       TVTCellPaintMode CellPaintMode,
                                       const System::Types::TRect &CellRect,
                                       System::Types::TRect &ContentRect);
    void __fastcall vstOnHeaderClick(TVTHeader* Sender, const TVTHeaderHitInfo &HitInfo);
  };
}; // namespace searcher

#endif
