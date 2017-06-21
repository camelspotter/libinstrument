#include "../include/parser.hpp"
#include "../include/util.hpp"

/**
	@file src/parser.cpp

	@brief Class instrument::parser method implementation
*/

namespace instrument {

/* Static member variable definition */

parser *parser::s_default = NULL;

style *parser::s_fallback = NULL;


/**
 * @brief Library constructor
 *
 * @attention
 *	If an exception occurs, its details are printed to the standard output and
 *	the process exits
 */
void parser::on_lib_load()
{
	try {
		/* Create the default parser */
		s_default = new parser;

		/*
		 * Equip the default parser with dictionaries for C++ keywords, intrinsic
		 * types and file extensions
		 */
		string path("%s/etc/extensions.dict", util::prefix());
		s_default->add_dictionary("extensions", path.cstring(), REGEXP_LOOKUP_MODE);

		path.set("%s/etc/keywords.dict", util::prefix());
		s_default->add_dictionary("keywords", path.cstring(), SIMPLE_LOOKUP_MODE);

		path.set("%s/etc/types.dict", util::prefix());
		s_default->add_dictionary("types", path.cstring(), SIMPLE_LOOKUP_MODE);

		/*
		 * Create the default, fallback style. When a highlighter can't determine or
		 * create/obtain the correct style for a token, it uses the fallback
		 */
		s_fallback = new style("fallback");

		/* Add styles for all kinds of trace tokens to the default parser */
		style *s = s_fallback->clone();
		s->set_name("delimiter");
		s_default->add_style(s);

		s = s_fallback->clone();
		s->set_name("file");
		s->set_fgcolor(HLT_FILE_FG);
		s_default->add_style(s);

		s = s_fallback->clone();
		s->set_name("function");
		s->set_fgcolor(HLT_FUNCTION_FG);
		s->set_attr_enabled(style::BOLD, true);
		s_default->add_style(s);

		s = s_fallback->clone();
		s->set_name("keyword");
		s->set_fgcolor(HLT_KEYWORD_FG);
		s_default->add_style(s);

		s = s_fallback->clone();
		s->set_name("number");
		s->set_fgcolor(HLT_NUMBER_FG);
		s->set_attr_enabled(style::BOLD, true);
		s_default->add_style(s);

		s = s_fallback->clone();
		s->set_name("scope");
		s->set_fgcolor(HLT_SCOPE_FG);
		s_default->add_style(s);

		s = s_fallback->clone();
		s->set_name("type");
		s->set_fgcolor(HLT_TYPE_FG);
		s->set_attr_enabled(style::BOLD, true);
		s_default->add_style(s);

		util::dbg_info("default C++ stack trace parser/highlighter initialized");
		return;
	}
	catch (exception &x) {
		std::cerr << x;
	}
	catch (std::exception &x) {
		std::cerr << x;
	}

	exit(EXIT_FAILURE);
}


/**
 * @brief Library destructor
 */
void parser::on_lib_unload()
{
	delete s_default;
	delete s_fallback;
	s_default = NULL;
	s_fallback = NULL;

	util::dbg_info("default C++ stack trace parser/highlighter finalized");
}


/**
 * @brief Stream insertion operator for instrument::parser objects
 *
 * @param[in] lval the output stream
 *
 * @param[in] rval the object to output
 *
 * @returns its first argument
 */
std::ostream& operator<<(std::ostream &lval, const parser &rval)
{
	util::lock();

	/* If an exception occurs, output its details instead of rval */
	try {
		string *buf = rval.highlight();
		lval << *buf;
		delete buf;
	}
	catch (exception &x) {
		lval << x;
	}
	catch (std::exception &x) {
		lval << x;
	}

	util::unlock();
	return lval;
}


/**
 * @brief Object default constructor
 *
 * @throws std::bad_alloc
 */
parser::parser()
try:
string(),
m_dictionaries(NULL),
m_styles(NULL)
{
	m_dictionaries = new chain<dictionary>;
	m_styles = new chain<style>;
}
catch (...) {
	delete[] m_data;
	m_data = NULL;

	delete m_dictionaries;
	m_dictionaries = NULL;
}


/**
 * @brief Object copy constructor
 *
 * @param[in] src the source object
 *
 * @throws std::bad_alloc
 */
parser::parser(const parser &src)
try:
string(src),
m_dictionaries(NULL),
m_styles(NULL)
{
	m_dictionaries = src.m_dictionaries->clone();
	m_styles = src.m_styles->clone();
}
catch (...) {
	delete[] m_data;
	m_data = NULL;

	delete m_dictionaries;
	m_dictionaries = NULL;
}


/**
 * @brief Object destructor
 */
parser::~parser()
{
	delete m_dictionaries;
	delete m_styles;
	m_dictionaries = NULL;
	m_styles = NULL;
}


/**
 * @brief Object virtual copy constructor
 *
 * @returns the object copy (heap allocated)
 *
 * @throws std::bad_alloc
 */
inline parser* parser::clone() const
{
	return new parser(*this);
}


/**
 * @brief Get the default (stack trace) parser
 *
 * @returns parser::s_default
 */
parser* parser::get_default()
{
	return s_default;
}


/**
 * @brief Get the shared fallback style
 *
 * @returns parser::s_fallback
 */
style* parser::get_fallback_style()
{
	return s_fallback;
}


/**
 * @brief Assignment operator
 *
 * @param[in] rval the assigned object
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 */
parser& parser::operator=(const parser &rval)
{
	if ( unlikely(this == &rval) ) {
		return *this;
	}

	/* Copy the buffer */
	string::operator=(rval);

	*m_dictionaries = *rval.m_dictionaries;
	*m_styles = *rval.m_styles;

	return *this;
}


/**
 * @brief Add a dictionary to the parser
 *
 * @param[in] nm the dictionary name
 *
 * @param[in] path the path of the dictionary data file
 *
 * @param[in] mode the lookup mode (REGEXP_LOOKUP_MODE or SIMPLE_LOOKUP_MODE)
 *
 * @returns the new dictionary (heap allocated)
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
dictionary* parser::add_dictionary(const i8 *nm, const i8 *path, bool mode)
{
	dictionary *retval = NULL;
	try {
		retval = new dictionary(nm, path, mode);
		m_dictionaries->add(retval);
		return retval;
	}
	catch (...) {
		delete retval;
		throw;
	}
}


/**
 * @brief Add a dictionary to the parser
 *
 * @param[in] dict the dictionary
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
inline parser& parser::add_dictionary(dictionary *dict)
{
	m_dictionaries->add(dict);
	return *this;
}


/**
 * @brief Get a dictionary, indexed by name
 *
 * @param[in] nm the dictionary name (can be NULL)
 *
 * @returns the dictionary or NULL if no such dictionary is registered
 */
dictionary* parser::get_dictionary(const i8 *nm) const
{
	__D_ASSERT(nm != NULL);
	if ( unlikely(nm == NULL) ) {
		return NULL;
	}

	for (u32 i = 0, sz = m_dictionaries->size(); likely(i < sz); i++) {
		dictionary *dict = m_dictionaries->at(i);

		if ( unlikely(strcmp(dict->name(), nm) == 0) ) {
			return dict;
		}
	}

	return NULL;
}


/**
 * @brief Get all the registered dictionary names
 *
 * @returns a list of names (heap allocated)
 *
 * @throws std::bad_alloc
 */
const chain<string>* parser::get_dictionary_names() const
{
	chain<string> *retval = new chain<string>;
	string *nm = NULL;

	try {
		for (u32 i = 0, sz = m_dictionaries->size(); likely(i < sz); i++) {
			nm = new string(m_dictionaries->at(i)->name());
			retval->add(nm);
			nm = NULL;
		}

		return retval;
	}
	catch (...) {
		delete retval;
		delete nm;
		throw;
	}
}


/**
 * @brief Remove all dictionaries
 *
 * @returns *this
 */
inline parser& parser::remove_all_dictionaries()
{
	m_dictionaries->clear();
	return *this;
}


/**
 * @brief Remove a dictionary, indexed by name
 *
 * @param[in] nm the dictionary name (can be NULL for NO-OP)
 *
 * @returns *this
 */
parser& parser::remove_dictionary(const i8 *nm)
{
	__D_ASSERT(nm != NULL);
	if ( unlikely(nm == NULL) ) {
		return *this;
	}

	for (u32 i = 0, sz = m_dictionaries->size(); likely(i < sz); i++) {
		const dictionary *dict = m_dictionaries->at(i);

		if ( unlikely(strcmp(dict->name(), nm) == 0) ) {
			m_dictionaries->remove(i);
			break;
		}
	}

	return *this;
}


/**
 * @brief Add a style to the parser/highlighter
 *
 * @param[in] nm the style name
 *
 * @param[in] fg the foreground color
 *
 * @param[in] bg the background color
 *
 * @param[in] set the text formatting attributes
 *
 * @returns the new style (heap allocated)
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
style* parser::add_style(const i8 *nm, color_t fg, color_t bg, attrset_t set)
{
	style *retval = NULL;
	try {
		retval = new style(nm, fg, bg, set);
		m_styles->add(retval);
		return retval;
	}
	catch (...) {
		delete retval;
		throw;
	}
}


/**
 * @brief Add a style to the parser/highlighter
 *
 * @param[in] stl the style
 *
 * @returns *this
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
inline parser& parser::add_style(style *stl)
{
	m_styles->add(stl);
	return *this;
}


/**
 * @brief Get a style, indexed by name
 *
 * @param[in] nm the style name (can be NULL to get the fallback style)
 *
 * @returns the style or parser::s_fallback if no such style is registered
 */
style* parser::get_style(const i8 *nm) const
{
	__D_ASSERT(nm != NULL);
	if ( unlikely(nm == NULL) ) {
		return s_fallback;
	}

	for (u32 i = 0, sz = m_styles->size(); likely(i < sz); i++) {
		style *stl = m_styles->at(i);

		if ( unlikely(strcmp(stl->name(), nm) == 0) ) {
			return stl;
		}
	}

	return s_fallback;
}


/**
 * @brief Get all the registered style names
 *
 * @returns a list of names (heap allocated)
 *
 * @throws std::bad_alloc
 */
const chain<string>* parser::get_style_names() const
{
	chain<string> *retval = new chain<string>;
	string *nm = NULL;

	try {
		for (u32 i = 0, sz = m_styles->size(); likely(i < sz); i++) {
			nm = new string(m_styles->at(i)->name());
			retval->add(nm);
			nm = NULL;
		}

		return retval;
	}
	catch (...) {
		delete retval;
		delete nm;
		throw;
	}
}


/**
 * @brief Remove all styles
 *
 * @returns *this
 */
inline parser& parser::remove_all_styles()
{
	m_styles->clear();
	return *this;
}


/**
 * @brief Remove a style, indexed by name
 *
 * @param[in] nm the style name (can be NULL for NO-OP)
 *
 * @returns *this
 */
parser& parser::remove_style(const i8 *nm)
{
	__D_ASSERT(nm != NULL);
	if ( unlikely(nm == NULL) ) {
		return *this;
	}

	for (u32 i = 0, sz = m_styles->size(); likely(i < sz); i++) {
		const style *stl = m_styles->at(i);

		if ( unlikely(strcmp(stl->name(), nm) == 0) ) {
			m_styles->remove(i);
			break;
		}
	}

	return *this;
}


/**
 * @brief Highlight (escape) the current buffer using a custom syntax
 *
 * @param[in] syntax a POSIX extended regular expression
 *
 * @param[in] icase true to ignore case while parsing
 *
 * @returns the escaped text (heap allocated)
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
string* parser::highlight(const i8 *syntax, bool icase) const
{
	const i8 *num = "^0x[0-9a-f]+$|^[0-9]+$";

	string *retval = new string;
	chain<string> *tokens = NULL;

	/* If an exception occurs, release resources and rethrow it */
	try {
		/* Parse the buffer */
		tokens = parse(syntax, icase);

		/* Highlight and append each token in the result */
		for (u32 i = 0, sz = tokens->size(); likely(i < sz); i++) {
			string *token = tokens->at(i);

			/* Select the style for the current token */
			const style *cur = NULL;
			if ( likely(i % 2 == 1) ) {
				cur = get_style("delimiter");
			}

			else if ( unlikely(token->match(num, true)) ) {
				cur = get_style("number");
			}

			else if ( unlikely(lookup(*token, "keywords")) ) {
				cur = get_style("keyword");
			}

			else if ( unlikely(lookup(*token, "types")) ) {
				cur = get_style("type");
			}

			/* Ignore case for extension (regexp) lookups */
			else if ( unlikely(lookup(*token, "extensions", true)) ) {
				cur = get_style("file");
			}

			/* Select the style based on the next delimiter */
			else if ( likely(i < sz - 1) ) {
				string *delim = tokens->at(i + 1);
				i8 ch = delim->at(0);

				if ( unlikely(delim->compare("::") == 0) ) {
					cur = get_style("scope");
				}

				else if ( unlikely(ch == '(' || ch == '<' || ch == '\r') ) {
					cur = get_style("function");
				}
			}

			/* If the token was not identified (plain text) */
			if ( unlikely(cur == NULL) ) {
				cur = s_fallback;
			}

			/* Apply the style to the token and append it to the result buffer */
			cur->apply(*token);
			retval->append(*token);
		}

		delete tokens;
		return retval;
	}
	catch (...) {
		delete tokens;
		delete retval;
		throw;
	}
}


/**
 * @brief Lookup an expression in one of the parser dictionaries
 *
 * @param[in] exp the expression
 *
 * @param[in] nm the dictionary name
 *
 * @param[in] icase true to ignore case in lookups
 *
 * @returns true if the expression is matched, false otherwise
 *
 * @throws instrument::exception
 */
inline bool parser::lookup(const string &exp, const i8 *nm, bool icase) const
{
	const dictionary *dict = get_dictionary(nm);
	if ( unlikely(dict == NULL) ) {
		return false;
	}

	return dict->lookup(exp, icase) != NULL;
}


/**
 * @brief Lookup an expression in all registered dictionaries
 *
 * @param[in] exp the expression
 *
 * @param[in] icase true to ignore case in lookups
 *
 * @returns the name of the first dictionary that found a match, NULL otherwise
 *
 * @throws instrument::exception
 */
const i8* parser::lookup(const string &exp, bool icase) const
{
	for (u32 i = 0, sz = m_dictionaries->size(); likely(i < sz); i++) {
		const dictionary *dict = m_dictionaries->at(i);

		if ( unlikely(dict->lookup(exp, icase) != NULL) ) {
			return dict->name();
		}
	}

	return NULL;
}


/**
 * @brief Parse the current buffer using a custom syntax
 *
 * @param[in] syntax a POSIX extended regular expression (can be NULL)
 *
 * @param[in] icase true to ignore case while parsing
 *
 * @returns the list of tokens (heap allocated)
 *
 * @throws std::bad_alloc
 * @throws instrument::exception
 */
inline chain<string>* parser::parse(const i8 *syntax, bool icase) const
{
	if ( likely(syntax == NULL) ) {
		syntax = g_trace_syntax;
	}

	return split(syntax, false, icase);
}

}
