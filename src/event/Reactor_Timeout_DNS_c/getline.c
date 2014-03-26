<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
<title>getline.c</title>
<style type="text/css">
.enscript-comment { font-style: italic; color: rgb(178,34,34); }
.enscript-function-name { font-weight: bold; color: rgb(0,0,255); }
.enscript-variable-name { font-weight: bold; color: rgb(184,134,11); }
.enscript-keyword { font-weight: bold; color: rgb(160,32,240); }
.enscript-reference { font-weight: bold; color: rgb(95,158,160); }
.enscript-string { font-weight: bold; color: rgb(188,143,143); }
.enscript-builtin { font-weight: bold; color: rgb(218,112,214); }
.enscript-type { font-weight: bold; color: rgb(34,139,34); }
.enscript-highlight { text-decoration: underline; color: 0; }
</style>
</head>
<body id="top">
<h1 style="margin:8px;" id="f1">getline.c&nbsp;&nbsp;&nbsp;<span style="font-weight: normal; font-size: 0.5em;">[<a href="?txt">plain text</a>]</span></h1>
<hr/>
<div></div>
<pre>
<span class="enscript-comment">/* getline.c -- Replacement for GNU C library function getline

Copyright (C) 1993 Free Software Foundation, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.  */</span>

<span class="enscript-comment">/* Written by Jan Brittenson, <a href="mailto:bson@gnu.ai.mit.edu">bson@gnu.ai.mit.edu</a>.  */</span>

#<span class="enscript-reference">ifdef</span> <span class="enscript-variable-name">HAVE_CONFIG_H</span>
#<span class="enscript-reference">include</span> <span class="enscript-string">&lt;config.h&gt;</span>
#<span class="enscript-reference">endif</span>

#<span class="enscript-reference">include</span> <span class="enscript-string">&lt;sys/types.h&gt;</span>
#<span class="enscript-reference">include</span> <span class="enscript-string">&lt;stdio.h&gt;</span>
#<span class="enscript-reference">include</span> <span class="enscript-string">&lt;assert.h&gt;</span>
#<span class="enscript-reference">include</span> <span class="enscript-string">&lt;errno.h&gt;</span>

#<span class="enscript-reference">if</span> <span class="enscript-variable-name">STDC_HEADERS</span>
#<span class="enscript-reference">include</span> <span class="enscript-string">&lt;stdlib.h&gt;</span>
#<span class="enscript-reference">else</span>
<span class="enscript-type">char</span> *<span class="enscript-function-name">malloc</span> (), *realloc ();
#<span class="enscript-reference">endif</span>

<span class="enscript-comment">/* Always add at least this many bytes when extending the buffer.  */</span>
#<span class="enscript-reference">define</span> <span class="enscript-variable-name">MIN_CHUNK</span> 64

<span class="enscript-comment">/* Read up to (and including) a TERMINATOR from STREAM into *LINEPTR
   + OFFSET (and null-terminate it). *LINEPTR is a pointer returned from
   malloc (or NULL), pointing to *N characters of space.  It is realloc'd
   as necessary.  Return the number of characters read (not including the
   null terminator), or -1 on error or EOF.  On a -1 return, the caller
   should check feof(), if not then errno has been set to indicate
   the error.  */</span>

<span class="enscript-type">int</span>
<span class="enscript-function-name">getstr</span> (lineptr, n, stream, terminator, offset)
     <span class="enscript-type">char</span> **lineptr;
     size_t *n;
     FILE *stream;
     <span class="enscript-type">char</span> terminator;
     <span class="enscript-type">int</span> offset;
{
  <span class="enscript-type">int</span> nchars_avail;		<span class="enscript-comment">/* Allocated but unused chars in *LINEPTR.  */</span>
  <span class="enscript-type">char</span> *read_pos;		<span class="enscript-comment">/* Where we're reading into *LINEPTR. */</span>
  <span class="enscript-type">int</span> ret;

  <span class="enscript-keyword">if</span> (!lineptr || !n || !stream)
    {
      errno = EINVAL;
      <span class="enscript-keyword">return</span> -1;
    }

  <span class="enscript-keyword">if</span> (!*lineptr)
    {
      *n = MIN_CHUNK;
      *lineptr = malloc (*n);
      <span class="enscript-keyword">if</span> (!*lineptr)
	{
	  errno = ENOMEM;
	  <span class="enscript-keyword">return</span> -1;
	}
    }

  nchars_avail = *n - offset;
  read_pos = *lineptr + offset;

  <span class="enscript-keyword">for</span> (;;)
    {
      <span class="enscript-type">int</span> save_errno;
      <span class="enscript-type">register</span> <span class="enscript-type">int</span> c = getc (stream);

      save_errno = errno;

      <span class="enscript-comment">/* We always want at least one char left in the buffer, since we
	 always (unless we get an error while reading the first char)
	 NUL-terminate the line buffer.  */</span>

      assert((*lineptr + *n) == (read_pos + nchars_avail));
      <span class="enscript-keyword">if</span> (nchars_avail &lt; 2)
	{
	  <span class="enscript-keyword">if</span> (*n &gt; MIN_CHUNK)
	    *n *= 2;
	  <span class="enscript-keyword">else</span>
	    *n += MIN_CHUNK;

	  nchars_avail = *n + *lineptr - read_pos;
	  *lineptr = realloc (*lineptr, *n);
	  <span class="enscript-keyword">if</span> (!*lineptr)
	    {
	      errno = ENOMEM;
	      <span class="enscript-keyword">return</span> -1;
	    }
	  read_pos = *n - nchars_avail + *lineptr;
	  assert((*lineptr + *n) == (read_pos + nchars_avail));
	}

      <span class="enscript-keyword">if</span> (ferror (stream))
	{
	  <span class="enscript-comment">/* Might like to return partial line, but there is no
	     place for us to store errno.  And we don't want to just
	     lose errno.  */</span>
	  errno = save_errno;
	  <span class="enscript-keyword">return</span> -1;
	}

      <span class="enscript-keyword">if</span> (c == EOF)
	{
	  <span class="enscript-comment">/* Return partial line, if any.  */</span>
	  <span class="enscript-keyword">if</span> (read_pos == *lineptr)
	    <span class="enscript-keyword">return</span> -1;
	  <span class="enscript-keyword">else</span>
	    <span class="enscript-keyword">break</span>;
	}

      *read_pos++ = c;
      nchars_avail--;

      <span class="enscript-keyword">if</span> (c == terminator)
	<span class="enscript-comment">/* Return the line.  */</span>
	<span class="enscript-keyword">break</span>;
    }

  <span class="enscript-comment">/* Done - NUL terminate and return the number of chars read.  */</span>
  *read_pos = <span class="enscript-string">'\0'</span>;

  ret = read_pos - (*lineptr + offset);
  <span class="enscript-keyword">return</span> ret;
}

<span class="enscript-type">int</span>
<span class="enscript-function-name">getline</span> (lineptr, n, stream)
     <span class="enscript-type">char</span> **lineptr;
     size_t *n;
     FILE *stream;
{
  <span class="enscript-keyword">return</span> getstr (lineptr, n, stream, <span class="enscript-string">'\n'</span>, 0);
}
</pre>
<hr />
</body></html>