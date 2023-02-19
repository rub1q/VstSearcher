#pragma hdrstop

#include "src/VstSearcher.h"
#include <algorithm>

#pragma package(smart_init)

namespace searcher {

template <typename T> ISet<T>&
__fastcall ISet<T>::operator<<(T&& value)
{
  add(std::forward<T>(value));
  return *this;
}

template <typename T> ISet<T>&
__fastcall ISet<T>::operator>>(T&& value)
{
  remove(std::forward<T>(value));
  return *this;
}

template <typename T>const auto& __fastcall ISet<T>::getData() const noexcept{  return data;}
template <typename T>
bool __fastcall ISet<T>::contains(T&& value) const
{
  return (data.find(std::forward<T>(value)) != data.end());
}

template <typename T>
bool __fastcall ISet<T>::empty() const
{
	return data.empty();
}

template <typename T>
void __fastcall ISet<T>::clear()
{
	data.clear();
}

TSearchOptions::TSearchOptions()
{
	*this << SearchOption::AUTO_EXPAND_NODES << SearchOption::RELEVANT_SORT;
}

void __fastcall TSearchOptions::add(SearchOption&& option)
{
  data.emplace(std::move(option));
}

void __fastcall TSearchOptions::remove(SearchOption&& option)
{
  data.erase(std::move(option));
}

void __fastcall TSearchColumns::add(unsigned&& iColumn)
{
  data.insert(std::move(iColumn));
}

void __fastcall TSearchColumns::remove(unsigned&& iColumn)
{
  data.erase(std::move(iColumn));
}

bool operator>(const Matches& lhs, const Matches& rhs)
{
  if ((lhs.totalMatches == rhs.totalMatches) &&
     (lhs.wordsMatches == rhs.wordsMatches))
  {
    return false;
  }
  else if ((lhs.totalMatches == rhs.totalMatches) &&
          (lhs.wordsMatches > rhs.wordsMatches))
  {
    return true;
  }
  else if ((lhs.totalMatches == rhs.totalMatches) &&
          (lhs.wordsMatches < rhs.wordsMatches))
  {
    return false;
  }
  else if ((lhs.totalMatches > rhs.totalMatches) &&
          (lhs.wordsMatches == rhs.wordsMatches))
  {
    return true;
  }
  else if ((lhs.totalMatches < rhs.totalMatches) &&
          (lhs.wordsMatches == rhs.wordsMatches))
  {
    return false;
  }
  else if ((lhs.totalMatches > rhs.totalMatches) &&
          (lhs.wordsMatches > rhs.wordsMatches))
  {
    return true;
  }
  else if ((lhs.totalMatches < rhs.totalMatches) &&
          (lhs.wordsMatches < rhs.wordsMatches))
  {
    return false;
  }
  else if ((lhs.totalMatches > rhs.totalMatches) &&
          (lhs.wordsMatches < rhs.wordsMatches))
  {
    return false;
  }
  else if ((lhs.totalMatches < rhs.totalMatches) &&
          (lhs.wordsMatches > rhs.wordsMatches))
  {
    return true;
  }

  return false;
}

bool operator<(const Matches& lhs, const Matches& rhs)
{
  if ((lhs.totalMatches == rhs.totalMatches) &&
      (lhs.wordsMatches == rhs.wordsMatches))
  {
    return false;
  }

  return !(operator>(lhs, rhs));
}

Matches& Matches::operator+=(const Matches& rhs)
{
  if (rhs.totalMatches > 0)
    totalMatches += rhs.totalMatches;

  if (rhs.wordsMatches > 0)
    wordsMatches += rhs.wordsMatches;

  return *this;
}

void __fastcall ISearcher::edtOnChange(TObject *Sender)
{
  if (TEditDefaultOnChange)
    TEditDefaultOnChange(Sender);

  edt_->RightButton->Visible = !edt_->Text.IsEmpty();

  if (edt_->Text.IsEmpty())
    ResetSearchResults();
}

void __fastcall ISearcher::edtOnKeyPress(TObject *Sender, System::WideChar &Key)
{
  if (TEditDefaultOnKeyPress)
    TEditDefaultOnKeyPress(Sender, Key);

  // Get rid off sound after press 'Enter'
	if (Key == VK_RETURN)
    Key = 0x0;
}

void __fastcall ISearcher::edtOnKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
  if (TEditDefaultOnKeyUp)
    TEditDefaultOnKeyUp(Sender, Key, Shift);

  // Allow enter only nums, letters, chars and Backspace
  if ((!std::isgraph(Key) && (Key != VK_BACK)) || (Key >= VK_F1 && Key <= VK_F24))
  {
    Key = 0x0;
    return;
  }

  if (edt_->Text.IsEmpty())
    return;

  bool requestIsTooShort = (static_cast<unsigned>(edt_->Text.Length()) < minRequestLen_);
  bool requestIsTooLong  = (static_cast<unsigned>(edt_->Text.Length()) > maxRequestLen_);

  if (requestIsTooShort || requestIsTooLong)
  {
    String errMsg;

    if (requestIsTooShort)
    {
      errMsg = "Minimal length of search request is: " +
                String(minRequestLen_) + "\r\n";
    }
    else
    {
      errMsg = "The maximum length of search request has been exceeded (" +
                String(maxRequestLen_)+ ").\r\n"
                "Try to reduce the number of characters\r\n";
    }

    ShowPopupMessage(std::move(errMsg));
    return;
  }

  if (edt_->CustomHint->ShowingHint)
    edt_->CustomHint->HideHint();

  if (!SearchOptions.contains(SearchOption::START_SEARCH_AFTER_BUTTON_CLICK))
  {
    // When you continuously enter a search request there are
    // interface freezes could  appear, so a delay (in ms) is created
    // before performing calculations

    if (timer_.isActive())
      timer_.reset();

    timer_.start(inputDelay_, this);
  }
}

bool __fastcall DelayTimer::isActive() const noexcept
{
  return timer_ != 0;
}

void __fastcall DelayTimer::reset() noexcept
{
  KillTimer(nullptr, timer_);
  timer_ = 0;
}

void __fastcall DelayTimer::start(const unsigned delay, ISearcher* const owner)
{
  owner_ = owner;
  timer_ = SetTimer(nullptr, 0, delay, TimerProc);
}

void __stdcall DelayTimer::TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
  if (uMsg == WM_TIMER && idEvent == timer_)
  {
    if (owner_)
      owner_->ProcessRequest();
  }
}

void __fastcall ISearcher::SetLabelCaption(String&& caption) noexcept
{
	if (!lbl_) return;

  lbl_->Caption = std::move(caption);
}

void __fastcall ISearcher::edtOnRightButtonClick(TObject *Sender)
{
  if (TEditDefaultOnRightButtonClick)
    TEditDefaultOnRightButtonClick(Sender);

  edt_->Clear();
  ResetSearchResults();
}

void __fastcall ISearcher::ShowPopupMessage(String&& msg) noexcept
{
  if (!edt_) return;

  edt_->CustomHint->Title = "Warning";
  edt_->CustomHint->ImageIndex = 2;

  TPoint p(edt_->Width / 2, edt_->Height);

  edt_->CustomHint->Description = std::move(msg);
  edt_->CustomHint->ShowHint(edt_->ClientToScreen(p));
}

__fastcall ISearcher::~ISearcher()
{
  if (edt_ && edt_->CustomHint)
    edt_->CustomHint->Free();
}

void __fastcall ISearcher::Init(TButtonedEdit *Edit, TLabel *Label)
{
  edt_ = Edit;
  lbl_ = Label;

  edt_->DoubleBuffered = true;

  if (edt_->CustomHint)
    edt_->CustomHint->Free();

  // Creating baloon hint for the search line
  TBalloonHint* pBHint = new TBalloonHint(edt_);

  pBHint->Style = bhsStandard;
  pBHint->Delay = 100;
  pBHint->HideAfter = 2000;

  edt_->CustomHint = pBHint;

  void __fastcall (__closure *const TEditOnChange)(TObject *Sender)                              = edt_->OnChange;
  void __fastcall (__closure *const TEditOnKeyPress)(TObject *Sender, System::WideChar &Key)  	 = edt_->OnKeyPress;
  void __fastcall (__closure *const TEditOnKeyUp)(TObject *Sender, WORD &Key, TShiftState Shift) = edt_->OnKeyUp;
  void __fastcall (__closure *const TEditOnRightButtonClick)(TObject *Sender)                    = edt_->OnRightButtonClick;

  TEditDefaultOnChange   = TEditOnChange;
  TEditDefaultOnKeyPress = TEditOnKeyPress;
  TEditDefaultOnKeyUp    = TEditOnKeyUp;
  TEditDefaultOnRightButtonClick = TEditOnRightButtonClick;

  // Redefining events
  edt_->OnChange 	 = edtOnChange;
  edt_->OnKeyPress = edtOnKeyPress;
  edt_->OnKeyUp 	 = edtOnKeyUp;
  edt_->OnRightButtonClick = edtOnRightButtonClick;
}

void __fastcall ISearcher::SetMinRequestLength(const unsigned min_len) noexcept
{
	minRequestLen_ = min_len;
}

void __fastcall ISearcher::SetMaxRequestLength(const unsigned max_len) noexcept
{
	maxRequestLen_ = max_len;
}

void __fastcall ISearcher::SetRequestLimits(const unsigned min_len, const unsigned max_len) noexcept
{
  SetMinRequestLength(min_len);
  SetMaxRequestLength(max_len);
}

void __fastcall ISearcher::SetInputDelay(const unsigned delay) noexcept
{
	inputDelay_ = delay;
}

void __fastcall ISearcher::AddWordsToList(const String& searchWords)
{
  words_.clear();

  std::string sWords = AnsiString(searchWords).c_str();

  static const char* delim = "./;,\t ";

  std::string::size_type start = 0,
                         end   = 0;

  while ((start = sWords.find_first_not_of(delim, end)) != std::string::npos)
  {
    end = sWords.find_first_of(delim, start);
    words_.insert(sWords.substr(start, end - start));
  }
}

Matches __fastcall ISearcher::CountMatches(const String& sText) const noexcept
{
  std::string text = AnsiString(sText).c_str();

  std::transform(text.begin(), text.end(), text.begin(), std::tolower);

  int iMatches = 0;
  int iWordsMatches = 0;

  for (const auto word : words_)
  {
    std::string searchWord = word;
    std::transform(searchWord.begin(), searchWord.end(), searchWord.begin(), std::tolower);

    std::string::size_type startSearchFrom = 0,
                           wordStartPos = 0;

    if (text.find(searchWord, startSearchFrom) != std::string::npos)
      iWordsMatches++;

    while ((wordStartPos = text.find(searchWord, startSearchFrom)) != std::string::npos)
    {
      iMatches += searchWord.length();
      startSearchFrom = wordStartPos + searchWord.length();
    }
  }

  return Matches(iMatches, iWordsMatches);
}

bool __fastcall ISearcher::WordsListEmpty() const noexcept
{
	return words_.empty();
}

std::size_t __fastcall ISearcher::WordsListSize() const noexcept
{
	return words_.size();
}

void __fastcall ISearcher::ClearWordsList() noexcept
{
	words_.clear();
}

int __fastcall ISearcher::CalculateTextWidth(HANDLE handle, const char* text) const
{
  RECT rect {0, 0, 0, 0};

  const std::size_t sz = strlen(text) + 1;
  wchar_t *txt = new wchar_t[sz];

  mbstowcs(txt, text, sz);

	DrawTextEx((HDC__*)handle, txt, wcslen(txt), &rect, DT_CALCRECT, 0);
  delete[] txt;

	return rect.right;
}

__fastcall VstSearcher::VstSearcher(TVirtualStringTree *Tree, TButtonedEdit *Edit, TLabel *Label)
{
	Init(Tree, Edit, Label);
}

void __fastcall VstSearcher::Init(TVirtualStringTree *Tree, TButtonedEdit *Edit, TLabel *Label)
{
  if (isInitialized_)
    return;

  if (!Tree || !Edit)
    throw Exception("Invalid arguments");

  setlocale(LC_ALL, "Russian_Russia.1251");

  ISearcher::Init(Edit, Label);

  vt_ = Tree;

  vt_->DoubleBuffered = true;

  void __fastcall (__closure *const TVTCompareEvent)(TBaseVirtualTree* Sender,
                                                     PVirtualNode Node1, PVirtualNode Node2,
                                                     TColumnIndex Column, int &Result)
  = vt_->OnCompareNodes;

  void __fastcall (__closure *const TVTBeforeCellPaintEvent)(TBaseVirtualTree* Sender,
                                                             Vcl::Graphics::TCanvas* TargetCanvas,
                                                             PVirtualNode Node, TColumnIndex Column,
                                                             TVTCellPaintMode CellPaintMode,
                                                             const System::Types::TRect &CellRect,
                                                             System::Types::TRect &ContentRect)
  = vt_->OnBeforeCellPaint;

  void __fastcall (__closure *const TVTHeaderClick)(TVTHeader* Sender,
                              const TVTHeaderHitInfo &HitInfo)
  = vt_->OnHeaderClick;

  TVTDefaultCompareEvent 		     = TVTCompareEvent;
  TVTDefaultBeforeCellPaintEvent = TVTBeforeCellPaintEvent;
  TVTDefaultHeaderClick 		     = TVTHeaderClick;

  vt_->OnBeforeCellPaint = vstOnBeforeCellPaint;
  vt_->OnHeaderClick     = vstOnHeaderClick;

  defaultSortColumn_ 	  = vt_->Header->SortColumn;
  defaultSortDirection_ = vt_->Header->SortDirection;

  isInitialized_ = true;

  ClearWordsList();
};

void __fastcall VstSearcher::ShowAllRecords() noexcept
{
  if (!vt_)
    return;

	vt_->BeginUpdate();

  for (auto Node = vt_->GetFirst(); Node != nullptr; Node = vt_->GetNext(Node))
  {
    if (!Node->States.Contains(vsVisible))
      vt_->IsVisible[Node] = true;
  }

  vt_->EndUpdate();
}

template <typename T>
static int CompareValues(const T& data_1, const T& data_2)
{
  if (data_1 > data_2)
    return 1;
  else if (data_1 < data_2)
    return -1;

  return 0;
}

void __fastcall VstSearcher::vstOnCompareNodes(TBaseVirtualTree *Sender, PVirtualNode Node1,
 								  			 PVirtualNode Node2, TColumnIndex Column, int &Result)
{
  const auto mit1 = matches_.find(Node1);
  const auto mit2 = matches_.find(Node2);

  if ((mit1 != matches_.end()) && (mit2 != matches_.end()))
    Result = CompareValues(mit1->second, mit2->second);
}

void __fastcall VstSearcher::vstOnBeforeCellPaint(TBaseVirtualTree* Sender,
                                                 Vcl::Graphics::TCanvas* TargetCanvas,
                                                 PVirtualNode Node, TColumnIndex Column,
                                                 TVTCellPaintMode CellPaintMode,
                                                 const System::Types::TRect &CellRect,
                                                 System::Types::TRect &ContentRect)
{
  if (TVTDefaultBeforeCellPaintEvent)
    TVTDefaultBeforeCellPaintEvent(Sender, TargetCanvas, Node, Column, CellPaintMode, CellRect, ContentRect);

	if (CellPaintMode == cpmPaint)
    HighlightTreeText(TargetCanvas, Node, Column, const_cast<TRect&>(CellRect));
}

void __fastcall VstSearcher::vstOnHeaderClick(TVTHeader* Sender, const TVTHeaderHitInfo &HitInfo)
{
	if (TVTDefaultHeaderClick)
    TVTDefaultHeaderClick(Sender, HitInfo);

  if (HitInfo.Button == mbLeft)
  {
    defaultSortColumn_ 	  = HitInfo.Column;
    defaultSortDirection_ = vt_->Header->SortDirection;
  }
}

void __fastcall VstSearcher::RelevantSort() noexcept
{
  if (!SearchOptions.contains(SearchOption::RELEVANT_SORT))
    return;

  if (!vt_) return;

  vt_->Header->SortColumn 	= -1;
  vt_->Header->SortDirection = sdDescending;

  vt_->SortTree(vt_->Header->SortColumn, vt_->Header->SortDirection);

  vt_->Header->SortColumn    = defaultSortColumn_;
  vt_->Header->SortDirection = defaultSortDirection_;
}

void __fastcall VstSearcher::ResetSearchResults()
{
  ShowAllRecords();
  vt_->FullCollapse();

  if (edt_->CustomHint->ShowingHint)
    edt_->CustomHint->HideHint();

  SetLabelCaption(vt_->TotalCount);

  vt_->Header->SortColumn 	 = defaultSortColumn_;
  vt_->Header->SortDirection = defaultSortDirection_;

  vt_->SortTree(vt_->Header->SortColumn, vt_->Header->SortDirection);
  vt_->ScrollIntoView(vt_->FocusedNode, true);

  ClearWordsList();
  matches_.clear();

  vt_->Repaint();
}

// This structure passes to the IterateSubTreeProc
struct TIterateData
{
  VstSearcher* searcher;
  Matches*     matches;

  explicit TIterateData(VstSearcher* const _searcher, Matches* const _matches)
      : searcher(_searcher)
      , matches(_matches)
  {};
};

void __fastcall IterateSubtreeProc(TBaseVirtualTree *Sender, PVirtualNode Node, void *Data, bool &Abort)
{
  if (!Data) return;

  TIterateData itData = *reinterpret_cast<TIterateData*>(Data);
  Matches m = itData.searcher->CountMatchesInNode(Node);

  const bool isAutoExpandNodes =
  itData.searcher->SearchOptions.contains(SearchOption::AUTO_EXPAND_NODES);

  if (isAutoExpandNodes && Sender->GetNodeLevel(Node) > 0)
  {
    // Expand all child nodes with matches
    // if AUTO_EXPAND_NODES option is specified

    if (m.totalMatches > 0)
    {
      TVirtualNode* ParentNode = Node->Parent;

      while (ParentNode && ParentNode != Sender->RootNode)
      {
        if (!ParentNode->States.Contains(vsExpanded))
          Sender->Expanded[ParentNode] = true;

        ParentNode = ParentNode->Parent;
      }
    }
    else // Collapse
    {
      if (Node->States.Contains(vsExpanded))
        Sender->Expanded[Node] = false;
    }
  }

  // Add to the common matches counter amount
  // of matches in the current node

	itData.matches->totalMatches += m.totalMatches;

  if (m.wordsMatches > itData.matches->wordsMatches)
    itData.matches->wordsMatches = m.wordsMatches;
}

void __fastcall VstSearcher::ProcessRequest()
{
  if (!vt_ || !edt_)
    throw Exception("Invalid arguments");

  if (edt_->Text.IsEmpty())
  {
    ResetSearchResults();
    return;
  }

  try
  {
    AddWordsToList(edt_->Text);

    vt_->OnCompareNodes = vstOnCompareNodes;

    TVirtualNode* Node = vt_->GetFirst();
    vt_->ScrollIntoView(Node, false);

    vt_->BeginUpdate();

    while (Node)
    {
      Matches m;
      TIterateData itD(this, &m);

      vt_->IterateSubtree(Node, new TVTGetNodeProcRef(IterateSubtreeProc), &itD);

      if (SearchOptions.contains(SearchOption::RELEVANT_SORT))
      {
        auto mit = matches_.find(Node);

        if (mit != matches_.end())
          mit->second = m;
        else
          matches_[Node] = m;
      }

      vt_->IsVisible[Node] = (m.totalMatches > 0);
      Node = vt_->GetNextSibling(Node);
    }

    RelevantSort();

    String caption;
    caption.printf(L"%d of %d", vt_->VisibleCount, vt_->TotalCount);

    SetLabelCaption(std::move(caption));
    vt_->EndUpdate();

    vt_->OnCompareNodes = TVTDefaultCompareEvent;
  }
  catch (Exception& e)
  {
    if (vt_->IsUpdating())
      vt_->EndUpdate();

    ClearWordsList();
    matches_.clear();

    Application->ShowException(&e);
  }
}

Matches __fastcall VstSearcher::CountMatchesInColumn(TVirtualNode* Node, const int colIndex) const
{
  if (colIndex >= vt_->Header->Columns->Count)
    throw Exception("Invalid search column index is specified");

  return ISearcher::CountMatches(vt_->Text[Node][colIndex]);
}

Matches __fastcall VstSearcher::CountMatchesInNode(TVirtualNode* Node) const
{
  if (!Node)
    throw Exception("Invalid arguments");

  Matches matches;

  if (!SearchColumns.empty())
  {
    for (auto it = SearchColumns.getData().cbegin(); it != SearchColumns.getData().cend(); it++)
      matches += CountMatchesInColumn(Node, *it);
  }
  else
  {
    matches = ISearcher::CountMatches(vt_->Text[Node][-1]);
  }

  return matches;
}

void __fastcall VstSearcher::HighlightTreeText(TCanvas* canvas, PVirtualNode Node,
											   TColumnIndex Column, TRect &CellRect) const
{
  // Don't highlight words in columns that wasn't added to the container
  if (!SearchColumns.empty() && !SearchColumns.contains(Column))
      return;

  if (!vt_) return;

  for (const auto& word : words_)
  {
    TFont* NodeFont = new TFont();
    TRect  displayRect;
    String nodeText;

    vt_->GetTextInfo(Node, Column, NodeFont, displayRect, nodeText);

    std::string strNodeText   = AnsiString(nodeText).c_str();
    std::string strSearchWord = word;

    canvas->Font = vt_->Font;

    bool isColumnFixed = false;

    if (Column >= 0)
      isColumnFixed = vt_->Header->Columns->Items[Column]->Options.Contains(coFixed);

    displayRect.Left += vt_->TextMargin - ((!isColumnFixed) ? vt_->OffsetX : 0);

    std::string searchWordUpperCase = strSearchWord;
    std::string nodeTextUpperCase   = strNodeText;

    std::transform(nodeTextUpperCase.begin(), nodeTextUpperCase.end(),
                   nodeTextUpperCase.begin(), std::tolower);
    std::transform(searchWordUpperCase.begin(), searchWordUpperCase.end(),
                   searchWordUpperCase.begin(), std::tolower);

    std::string::size_type wordStartPosInText = 0,
                           iStartSearchFrom   = 0;

    // The highlight logic is:
    // 1) Finding position of search word (strSearchWord) in node text (strNodeText);
    // 2) In order to determine the position CellRect.Left, we need to calculate the width
    //    of the area (uncoloredStringPart) that is up to the position of our word (wordStartPosInText);
    // 3) Determine the position of CellRect.Right like: position CellRect.Left + the width of colored area (coloredStringPart);
    // 4) Move iterator iStartSearchFrom by the value of the searched word length;
    // 5) Repeating (1-4) while having matches in node string.

    while ((wordStartPosInText = nodeTextUpperCase.find(searchWordUpperCase,
                                                        iStartSearchFrom)) != std::string::npos)
    {
        std::string uncoloredStringPart = strNodeText.substr(0, wordStartPosInText);
        std::string coloredStringPart   = strNodeText.substr(wordStartPosInText, strSearchWord.length());

        // To correctly highlight a word, it is necessary to take into account the text styles
        canvas->Font->Style = NodeFont->Style;

        CellRect.Left   = displayRect.Left + CalculateTextWidth(canvas->Handle, uncoloredStringPart.c_str());
        CellRect.Right  = CellRect.Left    + CalculateTextWidth(canvas->Handle, coloredStringPart.c_str());

        canvas->Brush->Color = TColor(0x73F1FF);
        canvas->TextRect(CellRect, CellRect.Left, CellRect.Right, coloredStringPart.c_str());

        iStartSearchFrom = wordStartPosInText + strSearchWord.length();
    }

    delete NodeFont;
  }
}
} // namespace searcher
