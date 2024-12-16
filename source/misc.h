﻿#ifndef MISC_H_INCLUDED
#define MISC_H_INCLUDED

#include <chrono>
#include <optional>
#include <string_view>
#include <vector>

#include <functional>
#include <fstream>
#include <mutex>
#include <atomic>
#include <sstream>
#include <queue>
#include <unordered_set>
#include <condition_variable>

#include "types.h"

// --------------------
//  engine info
// --------------------

// "USI"コマンドに応答するために表示する。
const std::string engine_info();

// 使用したコンパイラについての文字列を返す。
const std::string compiler_info();

// config.hで設定した値などについて出力する。
const std::string config_info();

// --------------------
//    prefetch命令
// --------------------

// prefetch()は、与えられたアドレスの内容をL1/L2 cacheに事前に読み込む。
// これはnon-blocking関数で、CPUがメモリに読み込むのを待たない。

void prefetch(const void* addr);

// --------------------
//  logger
// --------------------

// cin/coutへの入出力をファイルにリダイレクトを開始/終了する。
void start_logger(const std::string& fname);

// --------------------
//  統計情報
// --------------------

// bがtrueであった回数 / dbg_hit_on()が呼び出された回数 を調べるためのもの。
// (どれくらいの割合でXが成り立つか、みたいなのを調べるときに用いる)
void dbg_hit_on(bool b);

// if (c) dbg_hit_on(b)と等価。
void dbg_hit_on(bool c , bool b);

// vの合計 / 呼びだされた回数 ( = vの平均) みたいなのを求めるときに調べるためのもの。
void dbg_mean_of(int v);

// 探索部から1秒おきにdbg_print()が呼び出されるものとする。
// このとき、以下の関数を呼び出すと、その統計情報をcerrに出力する。
void dbg_print();

// --------------------
//  Time[ms] wrapper
// --------------------

// ms単位での時間計測しか必要ないのでこれをTimePoint型のように扱う。
typedef std::chrono::milliseconds::rep TimePoint;
static_assert(sizeof(TimePoint) == sizeof(int64_t), "TimePoint should be 64 bits");

// ms単位で現在時刻を返す
static TimePoint now() {
	return std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::steady_clock::now().time_since_epoch()).count();
		//(std::chrono::steady_clock::now().time_since_epoch()).count() * 10;
		// 10倍早く時間が経過するようにして、持ち時間制御のテストなどを行う。
}

// --------------------
//    HashTable
// --------------------

// このclass、Stockfishにあるんだけど、
// EvalHashとしてLargePageを用いる同等のclassをすでに用意しているので、使わない。

//template<class Entry, int Size>
//struct HashTable {
//	Entry* operator[](Key key) { return &table[(uint32_t)key & (Size - 1)]; }
//
//private:
//	std::vector<Entry> table = std::vector<Entry>(Size);
//};

// --------------------
//  sync_out/sync_endl
// --------------------

// Used to serialize access to std::cout
// to avoid multiple threads writing at the same time.

// スレッド排他しながらcoutに出力するために使う。
// 例)
// sync_out << "bestmove " << m << sync_endl;
// のように用いる。

enum SyncCout { IO_LOCK, IO_UNLOCK };
std::ostream& operator<<(std::ostream&, SyncCout);

#define sync_cout std::cout << IO_LOCK
#define sync_endl std::endl << IO_UNLOCK

// --------------------
//   from Stockfish
// --------------------

// Stockfish にあるけどやねうら王では使ってない。

//// align_ptr_up() : get the first aligned element of an array.
//// ptr must point to an array of size at least `sizeof(T) * N + alignment` bytes,
//// where N is the number of elements in the array.
//template <uintptr_t Alignment, typename T>
//T* align_ptr_up(T* ptr)
//{
//  static_assert(alignof(T) < Alignment);
//
//  const uintptr_t ptrint = reinterpret_cast<uintptr_t>(reinterpret_cast<char*>(ptr));
//  return reinterpret_cast<T*>(reinterpret_cast<char*>((ptrint + (Alignment - 1)) / Alignment * Alignment));
//}
//
//
//// IsLittleEndian : true if and only if the binary is compiled on a little endian machine
//static inline const union { uint32_t i; char c[4]; } Le = { 0x01020304 };
//static inline const bool IsLittleEndian = (Le.c[0] == 4);

// --------------------
//      ValueList
// --------------------

//  最大サイズが固定長のvectorみたいなやつ。
template<typename T, std::size_t MaxSize>
class ValueList {

public:
	std::size_t size() const { return size_; }
	void        push_back(const T& value) { values_[size_++] = value; }
	const T* begin() const { return values_; }
	const T* end() const { return values_ + size_; }

	const T& operator[](int index) const { return values_[index]; }
	// ⇨ ここの引数、どうせ大きな配列は確保しないのでsize_tではなくintで良い。

	// 非const版の begin/end(やねうら王独自追加)
	T* begin() { return values_; }
	T* end()   { return values_ + size_; }

private:
	T           values_[MaxSize];
	std::size_t size_ = 0;
};

// --------------------
//       乱数
// --------------------

// 擬似乱数生成器
// Stockfishで用いられているもの + 現在時刻によるseedの初期化機能。
// UniformRandomNumberGenerator互換にして、std::shuffle()等でも使えるようにするべきか？
struct PRNG
{
	PRNG(u64 seed) : s(seed) { ASSERT_LV1(seed); }

	// 時刻などでseedを初期化する。
	PRNG() {
		// C++11のrandom_device()によるseedの初期化
		// std::random_device rd; s = (u64)rd() + ((u64)rd() << 32);
		// →　msys2のgccでbuildすると同じ値を返すっぽい。なんぞこれ…。

		// time値とか、thisとか色々加算しておく。
		s = (u64)(time(NULL)) + ((u64)(this) << 32)
		//	+ (u64)(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		// ⇨ MSYS2 + clang18でhigh_resolution_clock::now()を使うとセグフォで落ちるようになった。
		//   代わりにsteady_clockを用いる。
			+ (u64)std::chrono::steady_clock::now().time_since_epoch().count();
	}

	// 乱数を一つ取り出す。
	template<typename T> T rand() { return T(rand64()); }

	// 0からn-1までの乱数を返す。(一様分布ではないが現実的にはこれで十分)
	u64 rand(u64 n) { return rand<u64>() % n; }

	// 内部で使用している乱数seedを返す。
	u64 get_seed() const { return s;  }

private:
	u64 s;
	u64 rand64() {
		s ^= s >> 12, s ^= s << 25, s ^= s >> 27;
		return s * 2685821657736338717LL;
	}
};

// 乱数のseedを表示する。(デバッグ用)
static std::ostream& operator<<(std::ostream& os, PRNG& prng)
{
	os << "PRNG::seed = " << std::hex << prng.get_seed() << std::dec;
	return os;
}

// --------------------
//  64bit×64bitの掛け算の上位64bitを取り出す関数
// --------------------

inline uint64_t mul_hi64(uint64_t a, uint64_t b) {
#if defined(__GNUC__) && defined(IS_64BIT)
    __extension__ using uint128 = unsigned __int128;
    return (uint128(a) * uint128(b)) >> 64;
#else
	// 64bit同士の掛け算を64bitを32bit 2つに分割して、筆算のようなことをするコード
    uint64_t aL = uint32_t(a), aH = a >> 32;
    uint64_t bL = uint32_t(b), bH = b >> 32;
    uint64_t c1 = (aL * bL) >> 32;
    uint64_t c2 = aH * bL + c1;
    uint64_t c3 = aL * bH + uint32_t(c2);
    return aH * bH + (c2 >> 32) + (c3 >> 32);
#endif
}

// --------------------
//  コマンドライン
// --------------------

struct CommandLine {
public:
	CommandLine(int _argc, char** _argv) :
		argc(_argc),
		argv(_argv) {}

	static std::string get_binary_directory(std::string argv0);
	static std::string get_working_directory();

	int    argc;
	char** argv;
};

// --------------------
//  全プロセッサを使う
// --------------------

// Windows環境において、プロセスが1個の論理プロセッサグループを超えてスレッドを
// 実行するのは不可能である。これは、最大64コアまでの使用に制限されていることを普通、意味する。
// これを克服するためには、いくつかの特殊なプラットフォーム固有のAPIを呼び出して、
// それぞのスレッドがgroup affinityを設定しなければならない。
// 元のコードはPeter ÖsterlundによるTexelから。

namespace WinProcGroup {
	// 各スレッドがidle_loop()などで自分のスレッド番号(0～)を渡す。
	// 1つ目のプロセッサをまず使い切るようにgroup affinityを割り当てる。
	// 1つ目のプロセッサの論理コアを使い切ったら次は2つ目のプロセッサを使っていくような動作。
	void bindThisThread(size_t idx);
}

// -----------------------
//  探索のときに使う時間管理用
// -----------------------

namespace Search { struct LimitsType; }

struct Timer
{
	// タイマーを初期化する。以降、elapsed()でinit()してからの経過時間が得られる。
	void reset() { startTime = startTimeFromPonderhit = now(); }

	// "ponderhit"からの時刻を計測する用
	void reset_for_ponderhit() { startTimeFromPonderhit = now(); }

	// 探索開始からの経過時間。単位は[ms]
	// 探索node数に縛りがある場合、elapsed()で探索node数が返ってくる仕様にすることにより、一元管理できる。
	TimePoint elapsed() const;

	// reset_for_ponderhit()からの経過時間。その関数は"ponderhit"したときに呼び出される。
	// reset_for_ponderhit()が呼び出されていないときは、reset()からの経過時間。その関数は"go"コマンドでの探索開始時に呼び出される。
	TimePoint elapsed_from_ponderhit() const;

	// reset()されてからreset_for_ponderhit()までの時間
	TimePoint elapsed_from_start_to_ponderhit() const { return (TimePoint)(startTimeFromPonderhit - startTime); }

#if 0
	// 探索node数を経過時間の代わりに使う。(こうするとタイマーに左右されない思考が出来るので、思考に再現性を持たせることが出来る)
	// node数を指定して探索するとき、探索できる残りnode数。
	// ※　StockfishでここintになっているのはTimePointにするのが正しいと思う。[2020/01/20]
	TimePoint availableNodes;
	// →　NetworkDelayやMinimumThinkingTimeなどの影響を考慮するのが難しく、将棋の場合、
	// 　相性があまりよろしくないのでこの機能はやねうら王ではサポートしないことにする。
#endif

	// このシンボルが定義されていると、今回の思考時間を計算する機能が有効になる。
#if defined(USE_TIME_MANAGEMENT)

	// 今回の思考時間を計算して、optimum(),maximum()が値をきちんと返せるようにする。
	// ※　ここで渡しているlimitsは、今回の探索の終わりまでなくならないものとする。
	//    "ponderhit"でreinit()でこの変数を参照することがあるため。
	void init(const Search::LimitsType& limits, Color us, int ply);

	// ponderhitの時に残り時間が付与されている時(USI拡張)、再度思考時間を調整するために↑のinit()相当のことを行う。
	void reinit() { init_(*lastcall_Limits, lastcall_Us, lastcall_Ply);}

	TimePoint minimum() const { return minimumTime; }
	TimePoint optimum() const { return optimumTime; }
	TimePoint maximum() const { return maximumTime; }

	// 1秒単位で繰り上げてdelayを引く。
	// ただし、remain_timeよりは小さくなるように制限する。
	TimePoint round_up(TimePoint t) const;

	// 探索終了の時間(startTime + search_end >= now()になったら停止)
	std::atomic<TimePoint> search_end;

private:
	TimePoint minimumTime;
	TimePoint optimumTime;
	TimePoint maximumTime;

	// Options["NetworkDelay"]の値
	TimePoint network_delay;
	// Options["MinimalThinkingTime"]の値
	TimePoint minimum_thinking_time;

	// 今回の残り時間 - Options["NetworkDelay2"]
	TimePoint remain_time;

	// init()の内部実装用。
	void init_(const Search::LimitsType& limits, Color us, int ply);

	// init()が最後に呼び出された時に各引数。これを保存しておき、reinit()の時にはこれを渡す。
	Search::LimitsType* lastcall_Limits; // どこかに確保しっぱなしにするだろうからポインタでいいや…
	Color lastcall_Us;
	int lastcall_Ply;

#endif

private:
	// 探索開始時刻。
	TimePoint startTime;

	// reset()かreset_for_ponderhit()が呼び出された時刻。
	TimePoint startTimeFromPonderhit;
};

extern Timer Time;


// =====   以下は、やねうら王の独自追加   =====

// --------------------
//  ツール類
// --------------------

namespace Tools
{
	// 進捗を表示しながら並列化してゼロクリア
	// Stockfishではtt.cppにこのコードがあるのだが、独自の置換表を確保したいときに
	// これが独立していないと困るので、ここに用意する。
	// nameは"Hash" , "eHash"などクリアしたいものの名前を書く。
	// メモリクリアの途中経過が出力されるときにその名前(引数nameで渡している)が出力される。
	// name == nullptrのとき、途中経過は表示しない。
	void memclear(const char* name, void* table, size_t size);

	// insertion sort
	// 昇順に並び替える。学習時のコードで使いたい時があるので用意してある。
	template <typename T >
	void insertion_sort(T* arr, int left, int right)
	{
		for (int i = left + 1; i < right; i++)
		{
			auto key = arr[i];
			int j = i - 1;

			// keyより大きな arr[0..i-1]の要素を現在処理中の先頭へ。
			while (j >= left && (arr[j] > key))
			{
				arr[j + 1] = arr[j];
				j = j - 1;
			}
			arr[j + 1] = key;
		}
	}

	// 途中での終了処理のためのwrapper
	// コンソールの出力が完了するのを待ちたいので3秒待ってから::exit(EXIT_FAILURE)する。
	void exit();

	// 指定されたミリ秒だけsleepする。
	void sleep(u64 ms);

	// 現在時刻を文字列化したもを返す。(評価関数の学習時などにログ出力のために用いる)
	std::string now_string();

	// Linux環境ではgetline()したときにテキストファイルが'\r\n'だと
	// '\r'が末尾に残るのでこの'\r'を除去するためにwrapperを書く。
	// そのため、ifstreamに対してgetline()を呼び出すときは、
	// std::getline()ではなくこのこの関数を使うべき。
	bool getline(std::ifstream& fs, std::string& s);

	// マルチバイト文字列をワイド文字列に変換する。
	// WindowsAPIを呼び出しているのでWindows環境専用。
	std::wstring MultiByteToWideChar(const std::string& s);

	// 他言語にあるtry～finally構文みたいなの。
	// SCOPE_EXIT()マクロの実装で使う。このクラスを直接使わないで。
	struct __FINALLY__ {
		__FINALLY__(std::function<void()> fn_) : fn(fn_) {}
		~__FINALLY__() { fn(); }
	private:
		std::function<void()> fn;
	};

	// --------------------
	//    ProgressBar
	// --------------------

	// 処理の進捗を0%から100%の間で出力する。
	class ProgressBar
	{
	public:
		ProgressBar(){}

		// size_ : 全件でいくらあるかを設定する。
		ProgressBar(u64 size_);

		// また0%に戻す。このインスタンスを再利用する時に用いる。
		void reset(u64 size_);

		// 進捗を出力する。
		// current : 現在までに完了している件数
		void check(u64 current);

		// Progress Barの有効/無効を切り替える。
		// "readyok"までにProgressBarが被るとよろしくないので
		// learnコマンドとmakebookコマンドの時以外はオフでいいと思う。
		static void enable(bool b) { enable_ = b; }

	private:
		// 全件の数。
		u64 size;

		// 前回までに何個dotを出力したか。
		size_t dots;

		static bool enable_;
	};

	// --------------------
	//  Result
	// --------------------

	// 一般的な関数の返し値のコード。(エラー理由などのenum)
	enum struct ResultCode
	{
		// 正常終了
		Ok,

		// ファイルの終端に達した
		Eof,

		// 原因の詳細不明。何らかのエラー。
		SomeError,

		// メモリ割り当てのエラー
		MemoryAllocationError,

		// ファイルが存在しないエラー。
		FileNotFound,

		// ファイルのオープンに失敗。
		FileOpenError,

		// ファイル読み込み時のエラー。
		FileReadError,

		// ファイル書き込み時のエラー。
		FileWriteError,

		// ファイルClose時のエラー。
		FileCloseError,

		// ファイルを間違えているエラー。
		FileMismatch,

		// フォルダ作成時のエラー。
		CreateFolderError,

		// 実装されていないエラー。
		NotImplementedError,
	};

	// ResultCodeを文字列化する。
	std::string to_string(ResultCode);

	// エラーを含む関数の返し値を表現する型
	// RustにあるOption型のような何か
	struct Result
	{
		constexpr Result(ResultCode code_) : code(code_) {}

		// エラーの種類
		ResultCode code;

		// 返し値が正常終了かを判定する
		bool is_ok() const { return code == ResultCode::Ok; }

		// 返し値が正常終了でなければtrueになる。
		bool is_not_ok() const { return code != ResultCode::Ok; }

		// 返し値がEOFかどうかを判定する。
		bool is_eof() const { return code == ResultCode::Eof; }

		// ResultCodeを文字列化して返す。
		std::string to_string() const { return Tools::to_string(code); }

		//  正常終了の時の型を返すbuilder
		static constexpr Result Ok() { return Result(ResultCode::Ok); }
	};
}

// スコープを抜ける時に実行してくれる。BOOST::BOOST_SCOPE_EXITマクロみたいな何か。
// 使用例) SCOPE_EXIT( x = 10 );
#define SCOPE_EXIT(STATEMENT) Tools::__FINALLY__ __clean_up_object__([&]{ STATEMENT });


// --------------------
//  ファイル操作
// --------------------

namespace SystemIO
{
	// ファイルを丸読みする。ファイルが存在しなくともエラーにはならない。空行はスキップする。末尾の改行は除去される。
	// 引数で渡されるlinesは空であるを期待しているが、空でない場合は、そこに追加されていく。
	// 引数で渡されるtrimはtrueを渡すと末尾のスペース、タブがトリムされる。
	// 先頭のUTF-8のBOM(EF BB BF)は無視する。
	Tools::Result ReadAllLines(const std::string& filename, std::vector<std::string>& lines, bool trim = false);

	// ファイルにすべての行を書き出す。
	Tools::Result WriteAllLines(const std::string& filename, std::vector<std::string>& lines);


	// msys2、Windows Subsystem for Linuxなどのgcc/clangでコンパイルした場合、
	// C++のstd::ifstreamで::read()は、一発で2GB以上のファイルの読み書きが出来ないのでそのためのwrapperである。
	// 	※　注意　どのみち32bit環境ではsize_tが4バイトなので2(4?)GB以上のファイルは書き出せない。
	//
	// read_file_to_memory()の引数のcallback_funcは、ファイルがオープン出来た時点でそのファイルサイズを引数として
	// callbackされるので、バッファを確保して、その先頭ポインタを返す関数を渡すと、そこに読み込んでくれる。
	//
	// また、callbackされた関数のなかでバッファが確保できなかった場合や、想定していたファイルサイズと異なった場合は、
	// nullptrを返せば良い。このとき、read_file_to_memory()は、読み込みを中断し、エラーリターンする。

	Tools::Result ReadFileToMemory(const std::string& filename, std::function<void* (size_t)> callback_func);
	Tools::Result WriteMemoryToFile(const std::string& filename, void* ptr, size_t size);

	// 通常のftell/fseekは2GBまでしか対応していないので特別なバージョンが必要である。

	size_t ftell64(FILE* f);
	int fseek64(FILE* f, size_t offset, int origin);

	// C#のTextReaderみたいなもの。
	// C++のifstreamが遅すぎるので、高速化されたテキストファイル読み込み器
	// fopen()～fread()で実装されている。
	struct TextReader
	{
		TextReader();
		virtual ~TextReader();

		// ファイルをopenする。
		Tools::Result Open(const std::string& filename);

		// Open()を呼び出してオープンしたファイルをクローズする。
		void Close();

		// ファイルの終了判定。
		// ファイルを最後まで読み込んだのなら、trueを返す。

		// 1行読み込む(改行まで) 引数のlineに代入される。
		// 改行コードは返さない。
		// SkipEmptyLine(),SetTrim()の設定を反映する。
		// Eofに達した場合は、返し値としてTools::ResultCode::Eofを返す。
		// 先頭のUTF-8のBOM(EF BB BF)は無視する。
		Tools::Result ReadLine(std::string& line);

		// ReadLine()で空行を読み飛ばすかどうかの設定。
		// (ここで行った設定はOpen()/Close()ではクリアされない。)
		// デフォルトでfalse
		void SkipEmptyLine(bool skip = true) { skipEmptyLine = skip; }

		// ReadLine()でtrimするかの設定。
		// 引数のtrimがtrueの時は、ReadLine()のときに末尾のスペース、タブはトリムする
		// (ここで行った設定はOpen()/Close()ではクリアされない。)
		// デフォルトでfalse
		void SetTrim(bool trim = true) { this->trim = trim; }

		// ファイルサイズの取得
		// ファイルポジションは先頭に移動する。
		size_t GetSize();

		// 現在のファイルポジションを取得する。
		// 先読みしているのでReadLineしている場所よりは先まで進んでいる。
		size_t GetFilePos() { return ftell64(fp); }

		// 現在の行数を返す。(次のReadLine()で返すのがテキストファイルの何行目であるかを返す) 0 origin。
		// またここで返す数値は空行で読み飛ばした時も、その空行を1行としてカウントしている。
		size_t GetLineNumber() const { return line_number; }

	private:
		// 各種状態変数の初期化
		void clear();

		// 次のblockのbufferへの読み込み。
		void read_next_block();

		// オープンしているファイル。
		// オープンしていなければnullptrが入っている。
		FILE* fp;

		// バッファから1文字読み込む。eofに達したら、-1を返す。
		int read_char();

		// ReadLineの下請け。何も考えずに1行読み込む。行のtrim、空行のskipなどなし。
		// line_bufferに読み込まれた行が代入される。
		Tools::Result read_line_simple();

		// ファイルの読み込みバッファ 1MB
		std::vector<u8> buffer;

		// 行バッファ
		std::vector<u8> line_buffer;

		// バッファに今回読み込まれたサイズ
		size_t read_size;

		// bufferの解析位置
		// 次のReadLine()でここから解析して1行返す
		// 次の文字 c = buffer[cursor]
		size_t cursor;

		// eofフラグ。
		// fp.eof()は、bufferにまだ未処理のデータが残っているかも知れないのでそちらを信じるわけにはいかない。
		bool is_eof;

		// 直前が\r(CR)だったのか？のフラグ
		bool is_prev_cr;

		// 何行目であるか
		// エラー表示の時などに用いる
		// 現在の行。(0 origin)
		size_t line_number;

		// ReadLine()で行の末尾をtrimするかのフラグ。
		bool trim;

		// ReadLine()で空行をskipするかのフラグ
		bool skipEmptyLine;
	};

	// Text書き出すの速いやつ。
	class TextWriter
	{
	public:
		// 書き出し用のバッファサイズ([byte])
		const size_t buf_size = 4096;

		Tools::Result Open(const std::string& filename);

		// 文字列を書き出す(改行コードは書き出さない)
		Tools::Result Write(const std::string& str);

		// 1行を書き出す(改行コードも書き出す) 改行コードは"\r\n"とする。
		Tools::Result WriteLine(const std::string& line);

		// ptrの指すところからsize [byte]だけ書き出す。
		Tools::Result Write(const char* ptr, size_t size);

		// 内部バッファにあってまだファイルに書き出していないデータをファイルに書き出す。
		// ※　Close()する時に呼び出されるので通常この関数を呼び出す必要はない。
		Tools::Result Flush();

		Tools::Result Close();
		TextWriter() : buf(buf_size) { clear(); }
		virtual ~TextWriter() { Close(); }

	private:
		// 変数を初期化する。
		void clear() { fp = nullptr; write_cursor = 0; }

		FILE* fp = nullptr;

		// 書き出し用のbuffer。これがいっぱいになるごとにfwriteする。
		std::vector<char> buf;

		// 書き出し用のcursor。次に書き出す場所は、buf[write_cursor]。
		size_t write_cursor;
	};

	// BinaryReader,BinaryWriterの基底クラス
	class BinaryBase
	{
	public:
		// ファイルを閉じる。デストラクタからClose()は呼び出されるので明示的に閉じなくても良い。
		Tools::Result Close();

		virtual ~BinaryBase() { Close(); }

	protected:
		FILE* fp = nullptr;
	};

	// binary fileの読み込みお手伝いclass
	class BinaryReader : public BinaryBase
	{
	public:
		// ファイルのopen
		Tools::Result Open(const std::string& filename);

		// ファイルサイズの取得
		// ファイルポジションは先頭に移動する。
		size_t GetSize();

		// ptrの指すメモリにsize[byte]だけファイルから読み込む
		// ファイルの末尾を超えて読み込もうとした場合、Eofが返る。
		// ファイルの末尾に超えて読み込もうとしなかった場合、Okが返る。
		// 引数で渡されたバイト数読み込むことができなかった場合、FileReadErrorが返る。
		// size_of_read_bytesがnullptrでない場合、実際に読み込まれたバイト数が代入される。
		// ※　sizeは2GB制限があるので気をつけて。
		Tools::Result Read(void* ptr , size_t size, size_t* size_of_read_bytes = nullptr);
	};

	// binary fileの書き出しお手伝いclass
	class BinaryWriter : public BinaryBase
	{
	public:
		// ファイルのopen
		// append == trueで呼び出すと、このあとWriteしたものはファイル末尾に追記される。
		Tools::Result Open(const std::string& filename, bool append = false);

		// ptrの指すメモリからsize[byte]だけファイルに書き込む。
		// ※　sizeは2GB制限があるので気をつけて。
		Tools::Result Write(void* ptr, size_t size);
	};
};

// Reads the file as bytes.
// Returns std::nullopt if the file does not exist.

// ファイルをバイトとして読み込みます。
// ファイルが存在しない場合は std::nullopt を返します。

std::optional<std::string> read_file_to_string(const std::string& path);

// --------------------
//       Path
// --------------------

// C#にあるPathクラス的なもの。ファイル名の操作。
// C#のメソッド名に合わせておく。
namespace Path
{
	// path名とファイル名を結合して、それを返す。
	// folder名のほうは空文字列でないときに、末尾に'/'か'\\'がなければそれを付与する。
	// 与えられたfilenameが絶対Pathである場合、folderを連結せずに単にfilenameをそのまま返す。
	// 与えられたfilenameが絶対Pathであるかの判定は、内部的にはPath::IsAbsolute()を用いて行う。
	//
	// 実際の連結のされ方については、UnitTestに例があるので、それも参考にすること。
	std::string Combine(const std::string& folder, const std::string& filename);

	// full path表現から、(フォルダ名をすべて除いた)ファイル名の部分を取得する。
	std::string GetFileName(const std::string& path);

	// full path表現から、(ファイル名だけを除いた)ディレクトリ名の部分を取得する。
	std::string GetDirectoryName(const std::string& path);

	// 絶対Pathであるかの判定。
	// ※　std::filesystem::absolute() は MSYS2 で Windows の絶対パスの判定に失敗するらしいので自作。
	//
	// 絶対Pathの条件 :
	//   "\\"(WindowsのUNC)で始まるか、"/"で始まるか(Windows / Linuxのroot)、"~"で始まるか、"C:"(ドライブレター + ":")で始まるか。
	//
	// 絶対Pathの例)
	//   C:/YaneuraOu/Eval  ← Windowsのドライブレター付きの絶対Path
	//   \\MyNet\MyPC\Eval  ← WindowsのUNC
	//   ~myeval            ← Linuxのhome
	//   /YaneuraOu/Eval    ← Windows、Linuxのroot
	bool IsAbsolute(const std::string& path);
};

// --------------------
//    Directory
// --------------------

// ディレクトリに存在するファイルの列挙用
// C#のDirectoryクラスっぽい何か
namespace Directory
{
	// 指定されたフォルダに存在するファイルをすべて列挙する。
	// 列挙するときに引数extensionで列挙したいファイル名の拡張子を指定できる。(例 : ".bin")
	// 拡張子として""を指定すればすべて列挙される。
	std::vector<std::string> EnumerateFiles(const std::string& sourceDirectory, const std::string& extension);

	// フォルダを作成する。
	// working directory相対で指定する。dir_nameに日本語は使っていないものとする。
	// ※　Windows環境だと、この関数名、WinAPIのCreateDirectoryというマクロがあって…。
	// 　ゆえに、CreateDirectory()をやめて、CreateFolder()に変更する。
	Tools::Result CreateFolder(const std::string& dir_name);

	// working directoryを返す。
	// "GetCurrentDirectory"という名前はWindowsAPI(で定義されているマクロ)と競合する。
	std::string GetCurrentFolder();
}

// --------------------
//    PRNGのasync版
// --------------------

// PRNGのasync版
struct AsyncPRNG
{
	// [ASYNC] 乱数を一つ取り出す。
	template<typename T> T rand() {
		std::unique_lock<std::mutex> lk(mutex);
		return prng.rand<T>();
	}

	// [ASYNC] 0からn-1までの乱数を返す。(一様分布ではないが現実的にはこれで十分)
	u64 rand(u64 n) {
		std::unique_lock<std::mutex> lk(mutex);
		return prng.rand(n);
	}

	// 内部で使用している乱数seedを返す。
	u64 get_seed() const { return prng.get_seed(); }

protected:
	std::mutex mutex;
	PRNG prng;
};

// 乱数のseedを表示する。(デバッグ用)
static std::ostream& operator<<(std::ostream& os, AsyncPRNG& prng)
{
	os << "AsyncPRNG::seed = " << std::hex << prng.get_seed() << std::dec;
	return os;
}

// --------------------
//       Parser
// --------------------

namespace Parser
{
	// スペースで区切られた文字列を解析するためのparser
	struct LineScanner
	{
		// 解析したい文字列を渡す(スペースをセパレータとする)
		LineScanner(std::string line_) : line(line_), pos(0) {}

		// 次のtokenを先読みして返す。get_token()するまで解析位置は進まない。
		std::string peek_text();

		// 次のtokenを返す。
		std::string get_text();

		// 現在のcursor位置から残りの文字列を取得する。
		// peek_text()した分があるなら、それも先頭にくっつけて返す。
		std::string get_rest();

		// 次の文字列を数値化して返す。
		// 空の文字列である場合は引数の値がそのまま返る。
		// "ABC"のような文字列で数値化できない場合は0が返る。(あまり良くない仕様だがatoll()を使うので仕方ない)
		s64 get_number(s64 defaultValue);
		double get_double(double defaultValue);

		// 解析位置(カーソル)が行の末尾まで進んだのか？
		// eolとはEnd Of Lineの意味。
		// get_text()をしてpeek_text()したときに保持していたものがなくなるまではこの関数はfalseを返し続ける。
		// このクラスの内部からeol()を呼ばないほうが無難。(token.empty() == trueが保証されていないといけないので)
		// 内部から呼び出すならraw_eol()のほうではないかと。
		bool eol() const { return token.empty() && raw_eol(); }

	private:
		// 解析位置(カーソル)が行の末尾まで進んだのか？(内部実装用)
		bool raw_eol() const { return !(pos < line.length()); }

		// 解析対象の行
		std::string line;

		// 解析カーソル(現在の解析位置)
		unsigned int pos;

		// peek_text()した文字列。get_text()のときにこれを返す。
		std::string token;
	};

	// PythonのArgumenetParserみたいなやつ
	// istringstream isを食わせて、そのうしろを解析させて、所定の変数にその値を格納する。
	// 使い方)
	//  isに"min 10 max 80"のような文字列が入っているとする。
	//
	//   ArgumentParser parser;
	//   int min=0,max=100;
	//   parser.add_argument("min",min);
	//   parser.add_argument("max",max);
	//   parser.parse_args(is);
	//
	// とすると min = 10 , max = 80となる。

	class ArgumentParser
	{
	public:
		typedef std::pair<std::string /*arg_name*/, std::function<void(std::istringstream&)>> ArgPair;

		// 引数を登録する。
		template<typename T>
		void add_argument(const std::string& arg_name, T& v)
		{
			auto f = [&](std::istringstream& is) { is >> v; };
			a.emplace_back(ArgPair(arg_name,f));
		}

		// 事前にadd_argument()で登録しておいた内容に基づき、isを解釈する。
		void parse_args(std::istringstream& is)
		{
			std::string token;
			while (is >> token)
				for (auto p : a)
					// 合致すれば次に
					if (p.first == token)
					{
						p.second(is);
						break;
					}
		}

		std::vector <ArgPair> a;
	};
}

// --------------------
//       Math
// --------------------

// 進行度の計算や学習で用いる数学的な関数
namespace Math {
	// シグモイド関数
	//  = 1.0 / (1.0 + std::exp(-x))
	double sigmoid(double x);

	// シグモイド関数の微分
	//  = sigmoid(x) * (1.0 - sigmoid(x))
	double dsigmoid(double x);

	// vを[lo,hi]の間に収まるようにクリップする。
	// ※　Stockfishではこの関数、bitboard.hに書いてある。
	template<class T> constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
		return v < lo ? lo : v > hi ? hi : v;
	}

	// cの倍数になるようにvを繰り上げる
	template<class T> constexpr const T align(const T v, const int c)
	{
		// cは2の倍数である。(1である一番上のbit以外は0
		ASSERT_LV3((c & (c - 1)) == 0);
		return (v + (T)(c - 1)) & ~(T)(c - 1);
	}

}

// --------------------
//    文字列 拡張
// --------------------

// 文字列拡張(やねうら王独自)
namespace StringExtension
{
	// 大文字・小文字を無視して文字列の比較を行う。
	// Windowsだと_stricmp() , Linuxだとstrcasecmp()を使うのだが、
	// 後者がどうも動作が怪しい。自前実装しておいたほうが無難。
	// stricmpは、string case insensitive compareの略？
	// s1==s2のとき0(false)を返す。
	bool stricmp(const std::string& s1, const std::string& s2);

	// 行の末尾の"\r","\n",スペース、"\t"を除去した文字列を返す。
	// ios::binaryでopenした場合などには'\r'なども入っていることがあるので…。
	std::string trim(const std::string& input);

	// trim()の高速版。引数で受け取った文字列を直接trimする。(この関数は返し値を返さない)
	void trim_inplace(std::string& input);

	// 行の末尾の数字を除去した文字列を返す。
	// sfenの末尾の手数を削除する用
	// 末尾のスペースを詰めたあと数字を詰めてそのあと再度スペースを詰める処理になっている。
	// 例 : "abc 123 "→"abc"となって欲しいので。
	std::string trim_number(const std::string& input);

	// trim_number()の高速版。引数で受け取った文字列を直接trimする。(この関数は返し値を返さない)
	void trim_number_inplace(std::string& s);

	// 文字列をint化する。int化に失敗した場合はdefault_の値を返す。
	int to_int(const std::string input, int default_);

	// スペース、タブなど空白に相当する文字で分割して返す。
	std::vector<std::string> split(const std::string& input);

	// 先頭にゼロサプライした文字列を返す。
	// 例) n = 123 , digit = 6 なら "000123"という文字列が返る。
	std::string to_string_with_zero(u64 n, int digit);

	// --- 以下、C#のstringクラスにあるやつ。

	// 文字列valueが、文字列endingで終了していればtrueを返す。
	bool StartsWith(std::string const& value, std::string const& starting);

	// 文字列valueが、文字列endingで終了していればtrueを返す。
	bool EndsWith(std::string const& value, std::string const& ending);

	// 文字列sのなかに文字列tが含まれるかを判定する。含まれていればtrueを返す。
	bool Contains(const std::string& s, const std::string& t);

	// 文字列valueに対して文字xを文字yに置換した新しい文字列を返す。
	std::string Replace(std::string const& value, char x, char y);

	// 文字列を大文字にして返す。
	std::string ToUpper(std::string const& value);

	// sを文字列spで分割した文字列集合を返す。
	std::vector<std::string_view> Split(std::string_view s, std::string_view delimiter);

	// Pythonの delemiter.join(v) みたいなの。
	// 例: v = [1,2,3] に対して ' '.join(v) == "1 2 3"
	std::string Join(const std::vector<std::string>& v , const std::string& delimiter);
};

// sを文字列spで分割した文字列集合を返す。
// ※ Stockfishとの互換性のために用意。
std::vector<std::string_view> split(std::string_view s, std::string_view delimiter);

// スペース相当文字列を削除する。⇨ NUMAの処理に必要
void remove_whitespace(std::string& s);

// スペース相当文字列かどうかを判定する。⇨ NUMAの処理に必要
bool is_whitespace(std::string_view s);

// "123"みたいな文字列を123のように数値型(size_t)に変換する。
size_t str_to_size_t(const std::string& s);

// --------------------
//    Concurrent
// --------------------

// 並列プログラミングでよく使うコンテナ類
namespace Concurrent
{
	// マルチスレッドプログラミングでよく出てくるProducer Consumer Queue
	template <typename T>
	class ConcurrentQueue
	{
	public:
		// [ASYNC] Queueのpop(一番最後にpushされた要素を取り出す)
		T pop()
		{
			std::unique_lock<std::mutex> lk(mutex_);

			// 要素がないなら待つしかない
			while (queue_.empty())
				cond_.wait(lk);

			auto val = queue_.front();
			queue_.pop();

			lk.unlock();
			cond_.notify_one();
			return val;
		}

		// [ASYNC] 先頭要素を返す。
		T& front() {
			// dequeは再配置しないことが保証されている。
			// そのためread-onlyで取得するだけならlock不要。
			return queue_.front();
		}

		// [ASYNC] Queueのpush(queueに要素を一つ追加する)
		void push(const T& item)
		{
			std::unique_lock<std::mutex> lk(mutex_);
			queue_.push(item);
			lk.unlock();
			cond_.notify_one();
		}

		// [ASYNC] Queueの保持している要素数を返す。
		size_t size()
		{
			std::unique_lock<std::mutex> lk(mutex_);
			return queue_.size();
		}

		// [ASYNC] Queueをclearする。
		void clear()
		{
			std::unique_lock<std::mutex> lk(mutex_);
			// clear by assignment
			queue_ = std::queue<T>();
		}

		// copyの禁止
		ConcurrentQueue() = default;
		ConcurrentQueue(const ConcurrentQueue&) = delete;

		// 代入の禁止
		ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;

	private:
		std::queue<T> queue_;
		std::mutex mutex_;
		std::condition_variable cond_;
	};

	// std::unordered_setの並列版
	template <typename T>
	class ConcurrentSet
	{
	public:
		// [ASYNC] Setのremove。
		void remove(const T& item)
		{
			std::unique_lock<std::mutex> lk(mutex_);
			set_.remove(item);
		}

		// [ASYNC] Setに要素を一つ追加する。
		void emplace(const T& item)
		{
			std::unique_lock<std::mutex> lk(mutex_);
			set_.insert(item);
		}

		// [ASYNC] Setに要素があるか確認する。
		bool contains(const T& item)
		{
			std::unique_lock<std::mutex> lk(mutex_);
			return set_.find(item) != set_.end();
		}

		// [ASYNC] Setの保持している要素数を返す。
		size_t size()
		{
			std::unique_lock<std::mutex> lk(mutex_);
			return set_.size();
		}

		// [ASYNC] Setをclearする。
		void clear()
		{
			std::unique_lock<std::mutex> lk(mutex_);
			// clear by assignment
			set_ = std::unordered_set<T>();
		}

		// copyの禁止
		ConcurrentSet() = default;
		ConcurrentSet(const ConcurrentSet&) = delete;

		// 代入の禁止
		ConcurrentSet& operator=(const ConcurrentSet&) = delete;

	private:
		std::unordered_set<T> set_;
		std::mutex mutex_;
	};
}

// --------------------
// StandardInputWrapper
// --------------------

// 標準入力のwrapper
// 事前にコマンドを積んだりできる。
class StandardInput
{
public:
	// 標準入力から1行もらう。Ctrl+Zが来れば"quit"が来たものとする。
	// また先行入力でqueueに積んでおくことができる。(次のinput()で取り出される)
	std::string input();

	// 先行入力としてqueueに積む。(次のinput()で取り出される)
	void push(const std::string& s);

	// main()に引数として渡されたパラメーターを解釈してqueueに積む。
	void parse_args(int argc, char* argv[]);

private:
	// 先行入力されたものを積んでおくqueue。
	// これが尽きれば標準入力から入力する。
	std::queue<std::string> cmds;
};

extern StandardInput std_input;

// --------------------
//     UnitTest
// --------------------

namespace Misc {
	// このheaderに書いてある関数のUnitTest。
	void UnitTest(Test::UnitTester& tester);
}

#endif // #ifndef MISC_H_INCLUDED
