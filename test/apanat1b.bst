%
% apanat1b.bst : a quick hack of apacite.bst
%                intended to work with natbib.sty and apanat1b.sty
% apacite.bst  : citation according to APA manual
% EXPERIMENTAL!
%
% author: Erik Meijer (E.Meijer@eco.rug.nl)
% date: 2003/03/27
%
% inspired by theapa.bst by Young U. Ryu
%
% documentation: not really available, but see apacite.tex, update.tex,
%                and the readme file
%
% NO GUARANTEES!
%
% If you change anything to this file, you must rename it.
%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%                                                           %
%           FIELD, INTEGER, AND STRING ENTRIES              %
%                                                           %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

ENTRY
  { address
    annote       % not used
    author
    booktitle    % for articles in books
    chair        % for lectures
    chapter      % not used
    day
    edition
    editor
    englishtitle % english translation of title in other language
    firstkey     % key for first citation
    howpublished
    institution  % for technical reports
    journal
    key          % key for second and later citations, or all
                 % citations if firstkey is missing
    month
    note
    number       % number of journal or report
    organization % for unpublished
        %
        % for reprints, translations, etc.
        %
    originaladdress
    originalbooktitle
    originaledition
    originaleditor
    originaljournal
    originalnumber
    originalpages
    originalpublisher
    originalvolume
    originalyear
        %
        % back to normal
        %
    pages
    publisher
    school       % for theses
    series       % not used
    symposium    % for lectures
    text         % for literals
    title
    translator   % translator of book (or article)
    type         % type of phdthesis (e.g., Doctoral dissertation)
                 % or type of article (e.g., Letter to the editor)
                 % etc.
    volume
    year
  }
  { title.number }
  { full.label
    label                  % label to be used in optional argument
                           % of \bibitem command
    year.label
    add.to.year.label      % for a, b, c, etc. after year
                           % in multiple citations with same author-year
    author.sort.label      % for sorting authors
    author.year.sort.label % for checking if a, b, c, etc. after year is
                           % is necessary
    title.sort.label       % for sorting titles
  }

    %
    % some to-be-used variables
    %
STRINGS { s t field }

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%                                                           %
%                        MONTH MACROS                       %
%                                                           %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


MACRO {jan} {"January"}

MACRO {feb} {"February"}

MACRO {mar} {"March"}

MACRO {apr} {"April"}

MACRO {may} {"May"}

MACRO {jun} {"June"}

MACRO {jul} {"July"}

MACRO {aug} {"August"}

MACRO {sep} {"September"}

MACRO {oct} {"October"}

MACRO {nov} {"November"}

MACRO {dec} {"December"}





%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%                                                           %
%            LOGICAL `NOT', `AND', AND `OR'                 %
%                                                           %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % if first element on stack is A
    % then and does the following:
    %     push { #0 }
    %     push { #1 }
    % so now first 3 elements are
    % { #1 } { #0 } A
    % first 3 are popped.
    % now if A > 0 then { #0 } is executed
    % else { #1 }  is executed:
    %     if A > 0
    %     then 0
    %     else 1
    % so consider integers as logicals, where 1=true
    % and 0 is false, then this does
    % (if A then false else true), which is a logical not
    %
FUNCTION {not}
{   { #0 }
    { #1 }
  if$
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % if first 2 elements on stack are A B
    % then and does the following:
    %     push 'skip$
    %     push { pop$ #0 }
    % so now first 4 elements are
    % { pop$ #0 } 'skip$ A B
    % first 3 are popped. B is on top of the stack.
    % now if A > 0 then 'skip is executed
    % else { pop$ #0 }  is executed:
    %     if A > 0
    % B stays on top of stack.
    %     else
    % now B is popped and #0 is pushed.
    % so consider integers as logicals, where 1=true
    % and 0 is false, then this does
    % (if A then B else false), which is a logical and
    %
FUNCTION {and}
{   'skip$
    { pop$ #0 }
  if$
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % if first 2 elements on stack are A B
    % then or does the following:
    %     push { pop$ #1 }
    %     push 'skip$
    % so now first 4 elements are
    % 'skip$ { pop$ #1 } A B
    % first 3 are popped. B is on top of the stack.
    % now if A > 0 then { pop$ #1 } is executed
    % else 'skip is executed:
    %     if A > 0
    % now B is popped and #1 is pushed.
    %     else
    % B stays on top of stack.
    % so consider integers as logicals, where 1=true
    % and 0 is false, then this does
    % (if A then true else B), which is a logical or
    %
FUNCTION {or}
{   { pop$ #1 }
    'skip$
  if$
}





%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%                                                           %
%           SPECIAL GENERAL PURPOSE FUNCTIONS               %
%                                                           %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % issues warning if field is empty
    % call with
    %    "field" warning.if.empty
    %
FUNCTION {warning.if.empty}
{ duplicate$ empty$
    { "No " swap$ * " in " * cite$ warning$ }
    { pop$ }
  if$
}




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % issues warning if title, type, and howpublished are empty
    %
FUNCTION {check.relevant.fields}
{ title empty$
    type empty$
      howpublished empty$
      and
    and
    { "No title, type, and howpublished in " cite$ * warning$ }
    'skip$
  if$
}




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % delivers 1 if (both editor and translator
    %                not empty and not equal to each other)
    %          0 if (editor or translator empty) or
    %               (editor = translator)
    %
FUNCTION {editor.ne.trans}
{ translator empty$
    { #0 }
    { editor empty$
        { #0 }
        { translator editor =
            { #0 }
            { #1 }
          if$
        }
      if$
    }
  if$
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % delivers 1 if possibly editor in author position
    %          2 if editor not in author position
    %          3 if unknown type
    %
FUNCTION {ref.type}
{ type$ "article" =
    type$ "magazine" =
      type$ "newspaper" =
        type$ "book" =
          type$ "techreport" =
            type$ "unpublished" =
              type$ "misc" =
                type$ "booklet" =
                  type$ "manual" =
                    type$ "proceedings" =
                    or
                  or
                or
              or
            or
          or
        or
      or
    or
    { #1 }
    { type$ "incollection" =
        type$ "phdthesis" =
          type$ "mastersthesis" =
            type$ "lecture" =
              type$ "inbook" =
                type$ "inproceedings" =
                  type$ "intechreport" =
                  or
                or
              or
            or
          or
        or
        { #2 }
        { type$ "literal" =
            { #3 }
            { #0 }
          if$
        }
      if$
    }
  if$
}



STRINGS { connect.string }

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % connects two strings with connect string
    % if one of them empty, then connect string
    % is left out
    %
    % call with S1 S2 connectstring connect.check
    %
FUNCTION {connect.check}
{ 'connect.string :=
    %
    % if S2 = ""
    %
  duplicate$ empty$
      %
      % then remove S2; S1 is on top of stack and is
      % the result that is given
      %
    { pop$ }
      %
      % else swap S1 and S2 so that S1 can be checked
      %
    { swap$
        %
        % if S1 is empty, remove S1 so that S2 is left on the
        % stack and is the result is given
        %
      duplicate$ empty$
        { pop$ }
            %
            % now the real work starts:
            % push the connect string "C"
            % so that top of stack is "C" "S1" "S2"
            % concatenate, so that top of stack is
            % "S1+C" "S2"
            %
        { connect.string *
            %
            % swap and concatenate
            %
          swap$ *
        }
      if$
    }
  if$
}




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % connects two strings with space ("\ ")
    % if length of second is 3 or larger,
    % connects them with non-breaking space ("tie", "~")
    % if length of second smaller than 3
    %
    % call with S1 S2 tie.or.space.connect
    % result: "S1\ S2" or "S1~S2"
    %
FUNCTION {tie.or.space.connect}
{ duplicate$ text.length$ #3 <
    { "~" }
    { "\ " }
  if$
  swap$ * *
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % connects two strings with comma (", ")
    % if one of them is empty, the comma is left out
    %
    % call with S1 S2 connect.with.comma.check
    % result: "S1, S2"
    %
FUNCTION {connect.with.comma.check}
{ ", " connect.check }

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % connects two strings with semicolon ("; ")
    % if one of them is empty, semicolon is left out
    %
    % call with S1 S2 connect.with.semicolon.check
    % result: "S1; S2"
    %
FUNCTION {connect.with.semicolon.check}
{ "; " connect.check }

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % connects two strings with colon (": ")
    % if one of them is empty, colon is left out
    %
    % call with S1 S2 connect.with.colon.check
    % result: "S1: S2"
    %
FUNCTION {connect.with.colon.check}
{ ": " connect.check }


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % connects two strings with space ("\ ")
    %
    % call with S1 S2 connect.with.space.check
    % result: "S1\ S2"
    %
FUNCTION {connect.with.space.check}
{ "\ " connect.check }



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % encloses string in pre- and postfix string
    % call with
    %    prefix postfix  S  enclose.check
    % delivers empty string if S empty
    %
FUNCTION {enclose.check}
{ duplicate$ empty$
    { pop$ pop$ pop$
      ""
    }
    { swap$ * * }
  if$
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % emphasizes top of stack
    % call with
    %     "string" emphasize.check
    %
FUNCTION {emphasize.check}
{ "\Bem{" swap$
  "}"     swap$
  enclose.check
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % brackets top of stack
    % call with
    %     "string" bracket
    %
FUNCTION {bracket.check}
{ "[" swap$
  "]" swap$
  enclose.check
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % parenthesizes top of stack
    % call with
    %     "string" parenthesize
    %
FUNCTION {parenthesize.check}
{ "(" swap$
  ")" swap$
  enclose.check
}






%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

INTEGERS { multiresult }

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % checks whether argument is "multiresult":
    % whether it contains '-', '+', or ',' characters
    % used with pages to check whether pp. or p. must be used
    %
FUNCTION {multi.result.check}
{ 't :=
  #0 'multiresult :=
  %
  % while (not multiresult) and (length(t) > 1) do
  %
    { multiresult not
      t text.length$ #1 >
      and
    }
        %
        % if t(1-2) = "--" or t(1) = "+" or ","
        % then  multiresult = 1
        % else  t = t(2-last)
        %
    { t #1 #2 substring$ 's :=
      "--" s =
        { #1 'multiresult := }
        { t #1 #1 substring$ 's :=
          "+" s =
            "," s =
            or
            { #1 'multiresult := }
            { t #2 global.max$ substring$ 't := }
          if$
        }
      if$
    }
  while$
  multiresult
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%                                                           %
%         GENERAL SORTING-LABELING HELP FUNCTIONS           %
%                                                           %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


INTEGERS {len}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % remove non-alphanumeric characters
    % and change to lower case
    %
FUNCTION {sortify}
{ purify$
  "l" change.case$
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % chop first N characters of string if they are
    % equal to teststring
    % call with
    %     teststring N string chop.word
    %
    % e.g.,
    %     "A " #2 "A fine old day" chop.word
    % gives "fine old day"
    % and
    %     "A " #2 "The fine old day" chop.word
    % gives "The fine old day"
    %
FUNCTION {chop.word}
{ 's :=
  'len :=
  s #1 len substring$ =
    { s len #1 + global.max$ substring$ }
    { s }
  if$
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % chop articles "A ", "An ", and "The "
    % call with
    %     string chop.articles
    %
FUNCTION {chop.articles}
{ 's :=
  "A " #2
    "An " #3
      "The " #4
        s
      chop.word
    chop.word
  chop.word
}









%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%                                                           %
%                BASIC OUTPUT FUNCTIONS                     %
%                                                           %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%






%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % starting new block by writing what's left on the stack,
    % starting a new line and adding some extra space or
    % whatever is more defined in \newblock
    %
FUNCTION {output.end.block}
{ write$
  newline$
}






%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % starting new block by writing what's left on the stack,
    % starting a new line and adding some extra space or
    % whatever is more defined in \newblock
    %
FUNCTION {output.new.block}
{ output.end.block
  "\newblock{}" write$
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % starting new block by writing what's left on the stack,
    % starting a new line and adding some extra space or
    % whatever is more defined in \newblock
    %
FUNCTION {output.dot.new.block}
{ add.period$
  output.new.block
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format \bibitem command with citation labels in
    % optional argument and cite key in mandatory
    % argument
    %
FUNCTION {output.bibitem}
{ newline$
  "\bibitem[\protect\citeauthoryear{" write$
  full.label                write$
  "}{"                      write$
  label                     write$
  "}{"                      write$
  year.label                write$
  "}]{"                     write$
  cite$                     write$
  "}"                       write$
  newline$
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % finish current entry
    % by writing what's left on the stack and starting a new line
    %
FUNCTION {fin.entry}
{ output.end.block }




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%                                                           %
%                     NAME FORMATTING                       %
%                                                           %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

INTEGERS { nameptr  namesleft  numnames  max.num.format
           name.max }


STRINGS { nameformat  between.names  between.two  before.last
          et.al. ampersand }

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % some name formats
    %

FUNCTION {author.name.format} { "{ll}{, f.}{ vv}{, jj}" }

FUNCTION {editor.name.format} { "{f.~}{vv~}{ll}{, jj}" }

FUNCTION {cite.name.format} { "{ll}" }

FUNCTION {sort.name.format} { "{ll{}}{  f{}}{  vv{}}{  jj{}}" }


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % some macro strings about commas, etc. between authors
    %
FUNCTION {comma.between.names} { ", " }

FUNCTION {space.between.names} { "   " }

FUNCTION {comma.between.two} { "\BCBT{}" }

FUNCTION {no.comma.between.two} { "" }

FUNCTION {comma.before.last} { "\BCBL{}" }

FUNCTION {no.comma.before.last} { "" }

FUNCTION {and.before.last} { " \BBA{} " }

FUNCTION {space.before.last} { "   " }

FUNCTION {et.al.string} { " \BOthers{}" }

FUNCTION {others.is.last} { "   zzz" }


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format a list of names
    %
    % call with
    %
    % between.names  between.two  before.last
    %   ampersand  et.al.  max.num.format
    %     nameformat  field  format.names
    %
FUNCTION {format.names}
{ 'field :=                       % field
  'nameformat :=                  % name format string
  'max.num.format :=              % maximum number of names to be
                                  % formatted
  'et.al. :=                      % string to be inserted for "others"
  'ampersand :=                   % "and" sign before last author
  'before.last :=                 % optional comma before last author
  'between.two :=                 % optional comma before last name
                                  % if #names = 2
  'between.names :=               % optional comma between other names

    %
    % begin with real work
    %
  #1 'nameptr :=                  % nameptr := 1; nameptr denotes
                                  %   which name is pointed at
  field num.names$ 'numnames :=   % numnames is the total number of names
                                  %   contained in field
  numnames 'namesleft :=          % namesleft is the number of names not
                                  %   formatted yet
    %
    % format first name and push on stack
    %
  field nameptr nameformat format.name$
  nameptr #1 + 'nameptr :=
  namesleft #1 - 'namesleft :=
    %
    % if num.names > max.num.format then
    %
  numnames max.num.format >
    { between.two * et.al. * }
      %
      % else
      %
      % while namesleft > 0
      %
    {
      { namesleft #0 > }
          %
          % do begin
          %
          % format name #nameptr in s according to specification
          % and assign result to t
          %
        { field nameptr nameformat format.name$ 't :=
            %
            %     if namesleft > 1
            %
          namesleft #1 >
                %
                % then
                %      % more than 1 name left
                %    add ", " to t and add to top of stack
                %
            { between.names * t * }
                %
                % else
                %      % 1 name left
                %    insert comma and "et al." or ampersand
                %    and last author
                %
            { numnames #2 >
                { before.last * }
                { between.two * }
              if$
              t "others" =
                { et.al. * }
                { ampersand * t * }
              if$
            }
                %
                % end if namesleft > 1
                %
          if$
            %
            % increase nameptr by 1 and decrease namesleft by 1
            %
          nameptr #1 + 'nameptr :=
          namesleft #1 - 'namesleft :=
        }
      while$
    }
  if$
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format label names (short cite)
    %
    % call with
    %    field format.lab.names
    %
FUNCTION {format.lab.names}
{ 'field :=
    %
  comma.between.names
    no.comma.between.two
      comma.before.last
        and.before.last
          et.al.string
            #2
              cite.name.format
                field
                  format.names
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format label names (full cite)
    %
    % call with
    %    field format.full.lab.names
    %
FUNCTION {format.full.lab.names}
{ 'field :=
    %
  comma.between.names
    no.comma.between.two
      comma.before.last
        and.before.last
          et.al.string
            #5
              cite.name.format
                field
                  format.names
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format names for sort key
    %
    % call with
    %    field format.sort.names
    %
FUNCTION {format.sort.names}
{ 'field :=
    %
  space.between.names
    no.comma.between.two
      no.comma.before.last
        space.before.last
          others.is.last
            name.max
              sort.name.format
                field
                  format.names
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format a list of names with the initials after the
    % last names (i.e., names in the author position),
    % commas between names, and ampersand (\&) before
    % last name
    %
    % call with
    %     field format.author.names
    %
FUNCTION {format.author.names}
{ 'field :=
    %
  comma.between.names
    comma.between.two
      comma.before.last
        and.before.last
          et.al.string
            name.max
              author.name.format
                field
                  format.names
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format a list of names with the initials before the
    % last names (i.e., names in the editor position),
    % commas between names (not between 2),
    % and ampersand (\&) before last name
    %
    % call with
    %     field format.editor.names
    %
FUNCTION {format.editor.names}
{ 'field :=
    %
  comma.between.names
    no.comma.between.two
      comma.before.last
        and.before.last
          et.al.string
            name.max
              editor.name.format
                field
                  format.names
}




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % "(Ed.)" postfix
    % editor must not be empty
    %
FUNCTION {editor.postfix}
{ editor num.names$ #1 >
    { "(\BEDS)" }
    { "(\BED)" }
  if$
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % "(Ed. and Trans.)" postfix
    % editor must not be empty
    %
FUNCTION {editor.trans.postfix}
{ editor num.names$ #1 >
    { "(\BEDS{} \BAnd{} \BTRANSS)" }
    { "(\BED{} \BAnd{} \BTRANS)" }
  if$
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % "(Trans.)" postfix
    % translator must not be empty
    %
FUNCTION {trans.postfix}
{ translator num.names$ #1 >
    { "(\BTRANSS)" }
    { "(\BTRANS)" }
  if$
}




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format authors, where it is given that the author field
    % is not empty: format names with initials after last
    % names
    %
FUNCTION {format.authors}
{ author format.author.names }



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format editors in case editors act as authors
    % (such as reference to entire edited book)
    %
    % Letheridge, S., \& Cannon, C. R. (Eds.).
    %     or
    % Letheridge, S. (Ed.).    (if only 1 editor)
    %
    % editor must not be empty
    %
FUNCTION {format.editors.as.authors}
{ editor           format.author.names
  editor.postfix   connect.with.space.check
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format editors in case editors are also translators
    % and act as authors
    % (such as reference to entire edited book of translated articles)
    %
    % Letheridge, S., \& Cannon, C. R. (Eds. and Trans.).
    %     or
    % Letheridge, S. (Ed. and Trans.).    (if only 1 editor)
    %
    % editor must not be empty
    %
FUNCTION {format.ed.trans.as.authors}
{ editor                 format.author.names
  editor.trans.postfix   connect.with.space.check
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format editors in case editors do not act as authors
    % (such as reference to chapter in edited book)
    %
    % L. W. Poon (Ed.)                         (1 editor)
    %    or
    % A. S. Gurman \& D. P. Kniskern (Eds.)    (2 editors)
    %    or
    % L. Poon, A. Jones, \& D. P. Smith (Eds.) (>2 editors)
    %
    % editor must not be empty
    %
FUNCTION {format.editors.in.line}
{ editor           format.editor.names
  editor.postfix   connect.with.space.check
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format editors in case editors are also translators
    % and do not act as authors
    % (such as reference to chapter in edited book
    %  of translated articles)
    %
    % L. W. Poon (Ed. and Trans.)                         (1 editor)
    %    or
    % A. S. Gurman \& D. P. Kniskern (Eds. and Trans.)    (2 editors)
    %    or
    % L. Poon, A. Jones, \& D. P. Smith (Eds. and Trans.) (>2 editors)
    %
    % editor must not be empty
    %
FUNCTION {format.editors.trans.in.line}
{ editor                 format.editor.names
  editor.trans.postfix   connect.with.space.check
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format translators in case translators act as editors
    % (such as reference to chapter in translated book)
    %
    % L. W. Poon (Trans.)                         (1 translator)
    %    or
    % A. S. Gurman \& D. P. Kniskern (Trans.)    (2 translators)
    %    or
    % L. Poon, A. Jones, \& D. P. Smith (Trans.) (>2 translators)
    %
    % translator must not be empty
    %
FUNCTION {format.translators.in.line}
{ translator      format.editor.names
  trans.postfix   connect.with.space.check
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format translators in case translator does not act as editor
    % (such as reference to translated book)
    %
    % L. Solotaroff, Trans.
    %
FUNCTION {format.translators.in.paren.check}
{ translator empty$
    { "" }
    { translator format.editor.names
      translator num.names$ #1 >
        { ", \BTRANSS{}" * }
        { ", \BTRANS{}" * }
      if$
    }
  if$
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % translator of article
    %
FUNCTION {format.atrans.check}
{ format.translators.in.paren.check   parenthesize.check }




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format "In editor (Ed.), "
    %
FUNCTION {format.in.editors}
{ "In "
  editor empty$
    'skip$
    { format.editors.in.line * ", " * }
  if$
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format "In translator (Trans.),"
    %
FUNCTION {format.in.trans}
{ "In "
  translator empty$
    'skip$
    { format.translators.in.line * ", " * }
  if$
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format "In editor (Ed. and Trans.)"
    %  or "In editor (Ed.)" if editor not equal to translator
    %
FUNCTION {format.in.editors.trans}
{ "In "
  editor empty$
    { translator empty$
        'skip$
        { format.translators.in.line * ", " * }
      if$
    }
    { translator empty$
        { format.editors.in.line * }
        { editor translator =
            { format.editors.trans.in.line * }
            { format.editors.in.line * }
          if$
        }
      if$
      ", " *
    }
  if$
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % citation label is author, editor, or key
    % push short and full cites on stack
    %
FUNCTION {author.editor.key.labels}
{ key empty$
    { author empty$
        { editor empty$
            { cite$
              duplicate$
            }
            { editor format.lab.names
              editor format.full.lab.names
            }
          if$
        }
        { author format.lab.names
          author format.full.lab.names
        }
      if$
    }
    { key
      firstkey empty$
        { key }
        { firstkey }
      if$
    }
  if$
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % citation label is author or key
    % push short and full cites on stack
    %
FUNCTION {author.key.labels}
{ key empty$
    { author empty$
        { cite$
          duplicate$
        }
        { author format.lab.names
          author format.full.lab.names
        }
      if$
    }
    { key
      firstkey empty$
        { key }
        { firstkey }
      if$
    }
  if$
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%                                                           %
%                            DATE                           %
%                                                           %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % checks if year present and formats year
    % (+ extra label) between parentheses (which
    % may be redefined by \BBOP and \BBCP).
    % extra label can be used for
    % Bentler, P. M. (1983a) <article 1>
    % Bentler, P. M. (1983b) <article 2>
    %
FUNCTION {format.year.check}
{ "year" warning.if.empty
  "\BBOP{}" "\BBCP{}"
  year add.to.year.label *   enclose.check
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % checks if year present and formats year
    % (+ extra label) + month + day between parentheses (which
    % may be redefined by \BBOP and \BBCP).
    % extra label can be used for
    % Bentler, P. M. (1983a) <article 1>
    % Bentler, P. M. (1983b) <article 2>
    %
FUNCTION {format.year.month.day.check}
{ "year" warning.if.empty
  "\BBOP{}"  "\BBCP{}"
  year  add.to.year.label *
  month connect.with.comma.check
  day   connect.with.space.check
  enclose.check
}




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format year for sort key
    %
    % call with
    %    format.sort.year
    %
FUNCTION {format.sort.year}
{ year empty$
    { "" }
    { year }
  if$
}






%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%                                                           %
%                     TITLE FORMATTING                      %
%                                                           %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % title of article in journal or book
    % no finishing period
    % field must be on top of stack
    %  (so englishtitle can be handled also)
    %
FUNCTION {format.atitle.no.dot}
{ "\BBOQ{}" swap$
  "\BBCQ{}" swap$
  duplicate$ empty$
    'skip$
    { "t" change.case$ }
  if$
  enclose.check
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % title of article in journal or book
    % with finishing period
    % field must be on top of stack
    %  (so englishtitle can be handled also)
    %
FUNCTION {format.atitle.dot}
{ "\BBOQ{}" swap$
  "\BBCQ{} " swap$
  duplicate$ empty$
    'skip$
    { "t" change.case$ add.period$ }
  if$
  enclose.check
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % title of article in journal or book
    % field must be on top of stack
    % second element on stack must be connected:
    %   if that is empty, then atitle.dot
    %   else atitle.no.dot, connect, and add period
    %
FUNCTION {format.atitle.connect}
{ swap$
  duplicate$ empty$
    { pop$
      format.atitle.dot
    }
    { swap$
      format.atitle.no.dot
      swap$ connect.with.space.check
      add.period$
    }
  if$
}




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % title of book or so, no period
    %
    % call with
    %     field format.btitle.no.dot
    %
FUNCTION {format.btitle.no.dot}
{ duplicate$ empty$
    'skip$
    { "t" change.case$ emphasize.check }
  if$
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % title of book or so, with period
    %     (maybe emphasize and add.period$ should be reversed)
    % call with
    %     field format.btitle.dot
    %
FUNCTION {format.btitle.dot}
{ duplicate$ empty$
    'skip$
    { "t" change.case$ add.period$ emphasize.check }
  if$
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % title of book and similar
    % field must be on top of stack
    % second element on stack must be connected:
    %   if that is empty, then btitle.dot
    %   else btitle.no.dot, connect, and add period
    %
FUNCTION {format.btitle.connect}
{ swap$
  duplicate$ empty$
    { pop$
      format.btitle.dot
    }
    { swap$
      format.btitle.no.dot
      swap$   connect.with.space.check
      add.period$
    }
  if$
}




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format englishtitle (of nonenglish article, etc.)
    %
FUNCTION {format.etitle.check}
{ englishtitle empty$
    { "" }
    { englishtitle "t" change.case$ bracket.check }
  if$
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format title and other non-name/date entries for sort key
    %
    % call with
    %    field format.sort.title
    %
FUNCTION {format.sort.title}
{ duplicate$ empty$
    { pop$
      ""
    }
    { chop.articles #1 entry.max$ substring$ }  % chop "A ", "An ", and "The "
  if$
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format type (of article, etc.)
    %
FUNCTION {format.atype.check}
{ type bracket.check }


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format title and englishtitle (of article, etc.)
    %
FUNCTION {format.atitle.check}
{ format.etitle.check
  title format.atitle.connect
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format title, englishtitle, and type
    % (of article, etc.)
    %
FUNCTION {format.atitle.type.check}
{ format.etitle.check
  format.atype.check   connect.with.space.check
  title format.atitle.connect
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format title, englishtitle,
    % and translator (of article, etc.)
    %
FUNCTION {format.atitle.trans.check}
{ format.etitle.check
  format.atrans.check  connect.with.space.check
  title format.atitle.connect
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format title, englishtitle, type,
    % and translator (of article, etc.)
    %
FUNCTION {format.atitle.type.trans.check}
{ format.etitle.check
  format.atype.check   connect.with.space.check
  format.atrans.check  connect.with.space.check
  title format.atitle.connect
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format journal, volume, number and pages
    % call with
    %   formatted.pages format.journal.vol.num.pages.check
    %
FUNCTION { format.journal.vol.num.pages.check }
{ "journal" warning.if.empty
  journal   emphasize.check
  volume    emphasize.check           connect.with.comma.check
  number    parenthesize.check  ""    connect.check
  swap$     connect.with.comma.check
}








%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % volume of book, checks whether
    % "Volume" or "Volumes"
    %
    % call with
    %   field format.bvolume.check
    %
FUNCTION {format.bvolume.check}
{ duplicate$ empty$
    'skip$
    { duplicate$ multi.result.check
        { "\BVOLS" swap$ tie.or.space.connect }
        { "\BVOL" swap$ tie.or.space.connect }
      if$
    }
  if$
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % edition of book, must not be empty
    % call with
    %   field format.edition.check
    %
FUNCTION {format.edition.check}
{ duplicate$ empty$
    'skip$
    { "\BEd" connect.with.space.check }
  if$
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % volume and edition of book
    %
FUNCTION {format.bvol.edition.check}
{ volume  format.bvolume.check
  edition format.edition.check connect.with.comma.check
}




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % volume and edition of book
    %
FUNCTION {format.bvol.edition}
{ format.bvol.edition.check
  parenthesize.check
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % volume and edition and translator of translated book
    %
FUNCTION {format.bvol.edition.trans}
{ format.bvol.edition.check
  format.translators.in.paren.check connect.with.semicolon.check
  parenthesize.check
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % formats pages by first checking if 1 or more pages
    % and prefixing pages with "p." or "pp." (these strings
    % are given in the commands \BPG and \BPGS, respectively),
    % whichever is applicable, and do a tie or space connect
    %
    % call with
    %   field format.bpages.check
    %
FUNCTION {format.bpages.check}
{ duplicate$ empty$
    'skip$
    { duplicate$ multi.result.check
        { "\BPGS" swap$ tie.or.space.connect }
        { "\BPG" swap$ tie.or.space.connect }
      if$
    }
  if$
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % volume, edition, and pages of article in book
    %
FUNCTION {format.bvol.edition.pages}
{ format.bvol.edition.check
  pages format.bpages.check connect.with.comma.check
  parenthesize.check
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % title, volume, edition of book
    %
FUNCTION {format.btitle.vol.edition}
{ format.etitle.check
  format.bvol.edition  connect.with.space.check
  title  format.btitle.connect
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % title, volume, edition, and translator of book
    %
FUNCTION {format.btitle.vol.edition.trans}
{ format.etitle.check
  format.bvol.edition.trans  connect.with.space.check
  title  format.btitle.connect
}




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format address and publisher of book etc.
    %
    % New York: Wiley
    %      or
    % Wiley      (if address empty)
    %
FUNCTION {format.address.publisher}
{ address publisher connect.with.colon.check }




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format address and publisher of book etc.
    %
    % New York: Wiley
    %      or
    % Wiley      (if address empty)
    %
    % warning if no publisher
    %
FUNCTION {format.address.publisher.check}
{ "publisher" warning.if.empty
  format.address.publisher
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format number as in reports: No. 2017
    % call with
    %   field format.rnumber.check
    %
FUNCTION {format.rnumber.check}
{ duplicate$ empty$
    'skip$
    { duplicate$ multi.result.check
        { "\BNUMS" swap$ tie.or.space.connect }
        { "\BNUM" swap$ tie.or.space.connect }
      if$
    }
  if$
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%                                                           %
%              NOTE FORMATTING FUNCTIONS                    %
%                                                           %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format optional note
    %
FUNCTION {format.note}
{ note parenthesize.check }




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format original year of publication and note
    % (reprint or translation)
    %
FUNCTION {format.orig.year.note}
{ "\BOWP{} " "" originalyear enclose.check      % original work published
  note connect.with.semicolon.check
  parenthesize.check
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format original journal of publication and note
    % (reprint or translation)
    %
FUNCTION {format.orig.journal.note}
{ "\BREPR{} "                                   % reprinted from
  originaljournal emphasize.check     *
  originalyear    connect.with.comma.check
  originalvolume  emphasize.check     connect.with.comma.check
  originalnumber  bracket.check ""    connect.check
  originalpages   connect.with.comma.check
  note            connect.with.semicolon.check
  parenthesize.check
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format original book of publication and note
    % (reprint or translation)
    %
FUNCTION {format.orig.book.note}
{ "\BREPR{} "
  originaleditor empty$
    'skip$
    { "originaleditor" format.editor.names *
      originaleditor num.names$ #1 >
        { ", \BEDS{}" }
        { ", \BED{}" }
      if$
    }
  if$
  originalyear       connect.with.comma.check
  originalbooktitle  format.btitle.no.dot  connect.with.comma.check
    %
    % volume, edition, pages
    %
  originalvolume  format.bvolume.check
  originaledition format.edition.check    connect.with.comma.check
  originalpages   format.bpages.check     connect.with.comma.check
  bracket.check   connect.with.space.check
    %
  originaladdress  originalpublisher  connect.with.colon.check
      connect.with.comma.check
  note                                connect.with.semicolon.check
  parenthesize.check
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format information about original publication of reprint
    % and format optional note
    %
FUNCTION {format.orig.note}
{ originaljournal empty$
    { originalbooktitle empty$
        { note empty$
            originalyear empty$
            and
            { add.period$ }
            { output.dot.new.block
              format.orig.year.note
            }
          if$
        }
        { output.dot.new.block
          format.orig.book.note
        }
      if$
    }
    { output.dot.new.block
      format.orig.journal.note
    }
  if$
}







%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%                                                           %
%                     PERIODICALS                           %
%                                                           %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % article in periodical: journal, magazine, newspaper, ...
    %
    % call with
    %   formatted.pages  formatted.date   periodical
    %
FUNCTION {periodical}
{ output.bibitem
    %
    % author or editor, year, title, englishtitle, type
    %
  author empty$
        %
        % no author: possibly special issue with editors as authors
        % (APA manual, ex. 11, p. 121)
        %
    { editor empty$
            %
            % no author or editor: title serves as author.
            % title formatted as article title (APA manual not consistent in this,
            % compare statement on p. 119: ``treat as book title'' and
            % example 8, p. 121: no underlining => article title)
            %
        { format.atitle.type.trans.check output.new.block
            %
            % now formatted date on top of stack
            %
          output.dot.new.block
        }
            %
            % format editor and year, and check if year is present
            % format article title and check if title or type present
            %
        { format.editors.as.authors output.dot.new.block
            %
            % now formatted date on top of stack
            %
          output.dot.new.block
          format.atitle.type.trans.check output.new.block
        }
      if$
    }
        %
        % format author and year, and check if year is present
        % format article title and check if title or type present
        %
    { format.authors output.dot.new.block
            %
            % now formatted date on top of stack
            %
      output.dot.new.block
      format.atitle.type.trans.check output.new.block
    }
  if$
    %
    % format journal name, volume and issue number, and pages
    %   and check if journal not empty
    %
  format.journal.vol.num.pages.check
    %
    % possible reprint and optional note
    %
  format.orig.note
  fin.entry
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % journal article
    %
FUNCTION {article}
{ pages  format.year.check  periodical }



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % magazine article
    %
FUNCTION {magazine}
{ pages format.bpages.check  format.year.month.day.check  periodical }



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % newspaper article
    %
FUNCTION {newspaper} {magazine}










%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % (translation of) entire book
    %
FUNCTION {book}
{ output.bibitem
      %
      % author or editor,
      % year, title, englishtitle, translator
      %
  author empty$
        %
        % no author: possibly edited book with editors as authors
        %
    { editor empty$
            %
            % no author or editor: title serves as author.
            % title formatted as book title
            %
        { format.btitle.vol.edition.trans  output.dot.new.block
          format.year.check                output.dot.new.block
        }
            %
            % format editor and year, and check if year is present
            % format booktitle, volume, and edition and check if title present
            %
        { translator empty$
            { format.editors.as.authors  output.dot.new.block
              format.year.check          output.dot.new.block
              format.btitle.vol.edition  output.dot.new.block
            }
                %
                % format translator correctly
                %
            { translator editor =
              not
                { format.editors.as.authors        output.dot.new.block
                  format.year.check                output.dot.new.block
                  "before title" cite$ * warning$
                  format.btitle.vol.edition.trans  output.dot.new.block
                }
                { format.ed.trans.as.authors  output.dot.new.block
                  format.year.check           output.dot.new.block
                  format.btitle.vol.edition   output.dot.new.block
                }
              if$
            }
          if$
        }
      if$
    }
        %
        % format author and year, and check if year is present
        % format booktitle, volume, and edition and check if title present
        %
    { format.authors                   output.dot.new.block
      format.year.check                output.dot.new.block
      format.btitle.vol.edition.trans  output.dot.new.block
    }
  if$
      %
      % format address and publisher, check if publisher present
      %
  format.address.publisher.check
      %
      % format original publication (of reprint/translation)
      % and optional note
      %
  format.orig.note
  fin.entry
}





%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % (translated) article or chapter in edited book
    %
FUNCTION {incollection}
{ output.bibitem
  author empty$
        %
        % no author: title serves as author.
        % title formatted as article title (APA manual not consistent in this,
        % compare statement on p. 119: ``treat as book title'' and
        % example 8, p. 121: no underlining => article title)
        %
    { editor.ne.trans
        { format.atitle.type.trans.check }
        { format.atitle.type.check }
      if$
      output.new.block
      format.year.check   output.dot.new.block
    }
        %
        % format author and year, and check if year is present
        % format article title and check if title or type present
        %
    { format.authors      output.dot.new.block
      format.year.check   output.dot.new.block
      editor.ne.trans
        { format.atitle.type.trans.check }
        { format.atitle.type.check }
      if$
      output.new.block
    }
  if$
      %
      % format "In " editor " (Ed. and Trans.), "
      % booktitle, volume, edition, pages
      %
  format.in.editors.trans
  format.bvol.edition.pages
  booktitle  format.btitle.connect * output.dot.new.block
      %
      % format address and publisher, check if publisher present
      %
  format.address.publisher.check
      %
      % format original publication (of reprint/translation)
      % and optional note
      %
  format.orig.note
  fin.entry
}





%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%                                                           %
%                       REPORTS                             %
%                                                           %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % technical report number
    %
FUNCTION {format.tr.number}
{ type empty$
    { "\BTR{}" }
    { type }
  if$
  number format.rnumber.check   connect.with.space.check
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % title, volume, edition, report number
    %
FUNCTION {format.tr.title.number}
{ format.etitle.check
  format.bvol.edition.check
  format.tr.number     connect.with.semicolon.check
  parenthesize.check   connect.with.space.check
  title  format.btitle.connect
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % format address and institution of reports etc.
    %
    % Washington, DC: National Institute of Education
    %      or
    % National Institute of Education (if address empty)
    %
    % warning if no institution
    %
FUNCTION {format.address.institution.check}
{ "institution" warning.if.empty
  address institution connect.with.colon.check
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % reports
    %
FUNCTION {techreport}
{ output.bibitem
  author empty$
        %
        % no author: possibly edited report with editors as authors
        %
    { editor empty$
            %
            % no author or editor: title serves as author.
            % title formatted as book title
            %
        { format.tr.title.number   output.dot.new.block
          format.year.check        output.dot.new.block
        }
            %
            % format editor and year, and check if year is present
            % format report title and check if title present
            % format volume, edition, type, and number
            %
        { format.editors.as.authors   output.dot.new.block
          format.year.check           output.dot.new.block
          format.tr.title.number      output.dot.new.block
        }
      if$
    }
        %
        % format author and year, and check if year is present
        % format report title and check if title present
        % format volume, edition, type, and number
        %
    { format.authors           output.dot.new.block
      format.year.check        output.dot.new.block
      format.tr.title.number   output.dot.new.block
    }
  if$
      %
      % format address and institution, check if institution present
      % format optional note
      %
  format.address.institution.check
  note empty$
    { add.period$ }
    { output.dot.new.block
      format.note
    }
  if$
  fin.entry
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % (translated) article or chapter in report
    %
FUNCTION {intechreport}
{ output.bibitem
  author empty$
        %
        % no author: title serves as author.
        % title formatted as article title (APA manual not consistent in this,
        % compare statement on p. 119: ``treat as book title'' and
        % example 8, p. 121: no underlining => article title)
        %
    { editor.ne.trans
        { format.atitle.trans.check }
        { format.atitle.check }
      if$
      output.new.block
      format.year.check output.dot.new.block
    }
        %
        % format author and year, and check if year is present
        % format article title and check if title or type present
        %
    { format.authors      output.dot.new.block
      format.year.check   output.dot.new.block
      editor.ne.trans
        { format.atitle.trans.check }
        { format.atitle.check }
      if$
      output.new.block
    }
  if$
      %
      % format "In " editor " (Ed. and Trans.), "
      % booktitle, volume, edition, pages
      %
  format.in.editors.trans
      %
      % volume, edition, report type and number, pages
      %
  format.bvol.edition.check
  format.tr.number             connect.with.semicolon.check
  pages   format.bpages.check  connect.with.comma.check
  parenthesize.check
      %
  booktitle format.btitle.connect *
  output.dot.new.block
      %
      % format address and publisher, check if publisher present
      %
  format.address.institution.check
      %
      % format original publication (of reprint/translation)
      % and optional note
      %
  format.orig.note
  fin.entry
}








%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % papers presented at conferences etc.
    %
FUNCTION {lecture}
{ output.bibitem
  "author" warning.if.empty
  format.authors output.dot.new.block
      %
      % format year, month, and day, and check if year present
      % format title and check if title present
      % format howpublished and check if howpublished present
      %
  format.year.month.day.check output.dot.new.block
  symposium empty$
    { format.etitle.check
      title  format.btitle.connect  output.new.block
      "howpublished" warning.if.empty
      howpublished
    }
    { format.atitle.type.check output.new.block
      "In "
      chair empty$
        'skip$
        { chair  format.editor.names
          chair num.names$ #1 >
            { "(\BCHAIRS)" }
            { "(\BCHAIR)" }
          if$
          connect.with.comma.check * ", " *
        }
      if$
      symposium format.btitle.no.dot *
      howpublished empty$
        'skip$
        { output.dot.new.block
          howpublished "t" change.case$
        }
      if$
    }
  if$
      %
      % format optional note
      %
  note empty$
    { add.period$ }
    { output.dot.new.block
      format.note
    }
  if$
  fin.entry
}





%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%                                                           %
%                        THESES                             %
%                                                           %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % default thesis type is on top of stack
    % now, if type field is empty, this default is used
    % else default is removed and replaced by value of
    % type field
    %
FUNCTION {format.type.school}
{ type empty$
    'skip$
    { pop$
      type "t" change.case$
    }
  if$
  school connect.with.comma.check
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % phd theses, etc.
    % call with
    %   "default.published.thesis" "default.unpublished.thesis" thesis
    %
FUNCTION {thesis}
{ output.bibitem
  "author" warning.if.empty
  format.authors output.dot.new.block
      %
      % format year and check if year present
      %
  format.year.check output.dot.new.block
      %
      % format title and check if title present
      % journal empty => unpublished PhD thesis
      % journal not empty => published (e.g., Dissertation Abstracts Int.)
      %
  journal empty$
    { format.btitle.vol.edition output.dot.new.block
        %
        % pop default unpublished thesis, top is default published thesis
        %
      swap$ pop$
      format.type.school
      address   connect.with.comma.check
    }
        %
        % pop default published thesis, top is default unpublished thesis
        %
    { pop$
      type empty$
        school empty$
          originalyear empty$
          and
        and
        { pop$
          ""
        }
        { format.type.school
          originalyear   connect.with.comma.check
          parenthesize.check
        }
      if$
      format.etitle.check
      swap$    connect.with.space.check
      "title"  warning.if.empty
      title    format.atitle.connect   output.new.block
      pages    format.journal.vol.num.pages.check
    }
  if$
      %
      % format optional note
      %
  note empty$
    { add.period$ }
    { output.dot.new.block
      format.note
    }
  if$
  fin.entry
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % phd theses, etc.
    %
FUNCTION {phdthesis}
{ "\BPhD"  "\BUPhD"  thesis }


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % master's theses, etc.
    %
FUNCTION {mastersthesis}
{ "\BMTh"  "\BUMTh"  thesis }







%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%                                                           %
%                        OTHER                              %
%                                                           %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % title of something (used in unpublished and misc)
    % english translation of title
    % type and number of something
    %
FUNCTION {format.btitle.type.num}
{ format.etitle.check
  number empty$
    { type bracket.check }
    { type
      number format.rnumber.check connect.with.space.check
      parenthesize.check
    }
  if$
  connect.with.space.check
  title empty$
    'skip$
    { title format.btitle.connect }
  if$
}





%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % unpublished manuscripts, etc.
    %
FUNCTION {unpublished}
{ output.bibitem
  check.relevant.fields
  author empty$
        %
        % no author: possibly edited book with editors as authors
        %
    { editor empty$
            %
            % no author or editor: title serves as author.
            % title formatted as book title
            %
        { format.btitle.type.num       output.dot.new.block
          format.year.month.day.check
        }
            %
            % format editor and year, and check if year is present
            % format book title, volume, and edition and check if title present
            %
        { format.editors.as.authors    output.dot.new.block
          format.year.month.day.check  output.dot.new.block
          format.btitle.type.num
        }
      if$
    }
        %
        % format author and year, and check if year is present
        % format book title, volume, and edition
        % and check if title or type present
        %
    { format.authors               output.dot.new.block
      format.year.month.day.check  output.dot.new.block
      format.btitle.type.num
    }
  if$
  howpublished empty$
    organization empty$
      address empty$
      and
    and
    'skip$
    { output.dot.new.block
      howpublished "t" change.case$
      organization    connect.with.comma.check
      address         connect.with.comma.check
    }
  if$
      %
      % format optional note
      %
  note empty$
    { add.period$ }
    { output.dot.new.block
      format.note
    }
  if$
  fin.entry
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % nonprint media and other things that don't fit
    %
FUNCTION {misc}
{ output.bibitem
  check.relevant.fields
  author empty$
        %
        % no author: possibly edited book with editors as authors
        %
    { editor empty$
            %
            % no author or editor: title/type serves as author.
            % title formatted as book title
            %
            % if no title and no type, howpublished is title
            %
        { title empty$
            type empty$
            and
            { howpublished "t" change.case$  output.dot.new.block
              format.year.month.day.check
            }
            { format.btitle.type.num  output.dot.new.block
              format.year.month.day.check
              howpublished empty$
                'skip$
                { output.dot.new.block
                  howpublished "t" change.case$
                }
              if$
            }
          if$
        }
            %
            % format editor and year, and check if year is present
            % format book title, volume, and edition and check if title present
            %
        { format.editors.as.authors    output.dot.new.block
          format.year.month.day.check  output.dot.new.block
          format.btitle.type.num
          howpublished empty$
            'skip$
            { output.dot.new.block
              howpublished "t" change.case$
            }
          if$
        }
      if$
    }
        %
        % format author and year, and check if year is present
        % format book title, volume, and edition
        % and check if title or type present
        %
    { format.authors               output.dot.new.block
      format.year.month.day.check  output.dot.new.block
      format.btitle.type.num
      howpublished empty$
        'skip$
        { output.dot.new.block
          howpublished
        }
      if$
    }
  if$
  address empty$
    publisher empty$
    and
    'skip$
    { output.dot.new.block
      format.address.publisher
    }
  if$
      %
      % format optional note
      %
  note empty$
    { add.period$ }
    { output.dot.new.block
      format.note
    }
  if$
  fin.entry
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % for legal cases and other cases that need to be
    % done by hand
    %
FUNCTION {literal}
{ output.bibitem
  "key"  warning.if.empty
  "text" warning.if.empty
  text
  fin.entry
}





%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % following functions not used for APA Manual examples
    %





%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % booklet
    %
FUNCTION {booklet}
{ misc }


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % part of a book
    %
FUNCTION {inbook}
{ incollection }


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % article in proceedings
    %
FUNCTION {inproceedings}
{ incollection }



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % technical documentation
    %
FUNCTION {manual}
{ misc }



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % proceedings
    %
FUNCTION {proceedings}
{ misc }


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % unknown types
    %
FUNCTION {default.type}
{ misc }










%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%  BASIC BIBLIOGRAPHY OUTPUT FUNCTIONS                      %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % beginning of the thebibliography environment:
    % write preamble if there is one
    % write \begin command
    %
FUNCTION {begin.bib}
{ preamble$ empty$
    'skip$
    { preamble$ write$ newline$ }
  if$
  "\begin{thebibliography}{}"  write$ newline$
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % write \end{thebibliography} command
    %
FUNCTION {end.bib}
{ newline$
  "\end{thebibliography}" write$
  newline$
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%  LABEL FORMATTING FUNCTIONS                               %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

INTEGERS {old.number}
%STRINGS {old.label}
FUNCTION {init.old.number}
{ #0 'old.number := }
FUNCTION {init1.old.number}
{ #1 'old.number := }
FUNCTION {init.old.label}
{ "000" 's := }



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % make title sorting label
    %
FUNCTION {make.sort.title}
{ title empty$
    { type empty$
        { howpublished  format.sort.title }
        { type          format.sort.title }
      if$
    }
    { title  format.sort.title }
  if$
  'title.sort.label :=
  title.sort.label 'sort.key$ :=
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % make title number: rank of sorted title
    %
FUNCTION {make.title.number}
{ old.number #1 + 'title.number :=
  title.number 'old.number :=
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % make author sort label and push on stack
    %
FUNCTION {author.editor.title.sort.label}
{ firstkey empty$
    { key empty$
        { author empty$
            { editor empty$
                { title empty$
                    { type  format.sort.title }
                    { title format.sort.title }
                  if$
                }
                { editor format.sort.names }
              if$
            }
            { author format.sort.names }
          if$
        }
        { key }
      if$
    }
    { firstkey }
  if$
  sortify
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % make author sort label and push on stack
    %
FUNCTION {author.title.sort.label}
{ firstkey empty$
    { key empty$
        { author empty$
            { title empty$
                { type  format.sort.title }
                { title format.sort.title }
              if$
            }
            { author format.sort.names }
          if$
        }
        { key }
      if$
    }
    { firstkey }
  if$
  sortify
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % make label on which authors are sorted
    %
FUNCTION {make.author.sort.label}
{ ref.type
  duplicate$ #1 =
    { pop$
      author.editor.title.sort.label
    }
    { duplicate$ #2 =
        { pop$
          author.title.sort.label
        }
        { #3 =
            { firstkey empty$
                { key empty$
                    { "key missing: " cite$ * warning$
                      " "
                    }
                    { key }
                  if$
                }
                { firstkey }
              if$
            }
            { "unknown type of reference: " cite$ * warning$
              author.editor.title.sort.label
            }
          if$
        }
      if$
    }
  if$
    %
    % now top element of stack is author sort label
    %
  #1 entry.max$ substring$
  'author.sort.label :=
  author.sort.label 'sort.key$ :=
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % convert integer to string, assume that integer < 1000
    %
FUNCTION {conv.int.to.str}
{ duplicate$ #10 <
    { "00" swap$ int.to.str$ * }
    { duplicate$ #100 <
        { "0" swap$ int.to.str$ * }
        { int.to.str$ }
      if$
    }
  if$
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % make label on which entries are sorted
    %
FUNCTION {make.sort.label}
{ s author.sort.label =
    'skip$
    { old.number #1 + 'old.number :=
      author.sort.label 's :=
    }
  if$
  old.number conv.int.to.str
  year "\BIP" =
    { "  9999  " * }
    { "  " * year * "  " * }
  if$
  sortify
  'author.year.sort.label :=
  author.year.sort.label
  title.number conv.int.to.str *
  'sort.key$ :=
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % make b, c, etc. labels for subsequent entries
    % with same author/year (forward pass)
    %
FUNCTION {check.add.to.year}
{ s author.year.sort.label =
    { old.number #1 + 'old.number :=
      "\protect"
      year "\BIP" =
        { "\BCntIP" * }
        { "\BCnt" * }
      if$
      "{" * old.number int.to.str$ * "}" *
      'add.to.year.label :=
    }
    { #1 'old.number :=
      author.year.sort.label 's :=
      "" 'add.to.year.label :=
    }
  if$
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % make "a" label for first entry of a number of entries
    % with same author/year (backward pass)
    %
FUNCTION {make.add.to.year}
{ s "b" =
    add.to.year.label "" =
    and
    { year "\BIP" =
        { "\protect\BCntIP{1}" 'add.to.year.label := }
        { "\protect\BCnt{1}" 'add.to.year.label := }
      if$
      "a" 's :=
    }
    { add.to.year.label "" = not
        { "b" 's := }
        { "a" 's := }
      if$
    }
  if$
}





%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % make year label and push on stack
    %
FUNCTION {make.year.label}
{ originalyear year "/" connect.check
  add.to.year.label *
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    % top make-label-function
    %
FUNCTION {make.labels}
{ ref.type
  duplicate$ #1 =
    { pop$
      author.editor.key.labels
    }
    { duplicate$ #2 =
        { pop$
          author.key.labels
        }
        { #3 =
            { key
              firstkey empty$
                { key      }
                { firstkey }
              if$
            }
            { author.editor.key.labels }
          if$
        }
      if$
    }
  if$
    %
    % now top element of stack is full cite label,
    % and second element is short cite label
    %
  'full.label :=
  'label      :=
  make.year.label 'year.label :=
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%  READ AND PROCESS ENTRIES, MAKE THEBIBLIOGRAPHY           %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    %
    % read entries
    %
READ


FUNCTION {initialize}
{ #100 'name.max :=
%  #200 'entry.max$ :=
%  #200 'global.max$ :=
}

EXECUTE { initialize }

    %
    % sort items by title in order to replace each title by
    % its number in later sorting
    %
ITERATE {make.sort.title}
SORT
EXECUTE {init.old.number}
ITERATE {make.title.number}



    %
    % author label on which items will be sorted
    %
ITERATE {make.author.sort.label}

    %
    % sort items by author (or substitute),
    %
SORT

    %
    % make definitive sort labels
    % and definitive sorting
    %
EXECUTE {init.old.number}
EXECUTE {init.old.label}
ITERATE {make.sort.label}
SORT

    %
    % make "a" and "b", etc. additions for
    % citations to several works with same
    % author-year combination
    %
EXECUTE {init.old.label}
EXECUTE {init1.old.number}
ITERATE {check.add.to.year}
EXECUTE {init.old.label}
REVERSE {make.add.to.year}

    %
    % make label (optional argument of \bibitem command)
    % for each entry
    %
ITERATE {make.labels}


    %
    % write preamble and \begin{thebibliography}{} statement
    %
EXECUTE {begin.bib}

    %
    % format and write entries
    %
ITERATE {call.type$}

    %
    % write \end{thebibliography} statement
    %
EXECUTE {end.bib}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                           %
%                                                           %
%  DONE                                                     %
%                                                           %
%                                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% EOF apanat1b.bst
%

