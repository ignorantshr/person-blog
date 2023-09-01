
```go
package main

import (
	"encoding/json"
	"log"
	"os"
	"path"
	"strings"
)

var w *writer = &writer{builder: new(strings.Builder)}

func init() {
	log.SetFlags(log.Lshortfile | log.Ltime)
}

func main() {
	if err := scanDir(os.Args[1]); err != nil {
		log.Fatalln(err)
	}
}

func scanDir(d string) error {
	dir, err := os.ReadDir(d)
	if err != nil {
		return err
	}

	for _, entry := range dir {
		p := path.Join(d, entry.Name())
		if entry.IsDir() {
			if err := scanDir(p); err != nil {
				return err
			}
		} else {
			if p[len(p)-3:] != ".km" {
				continue
			}
			content, err := os.ReadFile(p)
			if err != nil {
				return err
			}

			if err := convertDoc(p, content); err != nil {
				return err
			}
		}
	}
	return nil
}

func convertDoc(p string, content []byte) error {
	tree := struct {
		Root node `json:"root"`
	}{}
	if err := json.Unmarshal(content, &tree); err != nil {
		return err
	}

	root := tree.Root
	w.writeContent("[TOC]")
	if err := walkWrite(1, &root); err != nil {
		return err
	}

	f, err := os.OpenFile(path.Join(path.Dir(p), strings.Replace(path.Base(p), ".km", ".md", 1)), os.O_CREATE|os.O_TRUNC|os.O_WRONLY, 0644)
	if err != nil {
		return err
	}
	defer f.Close()

	_, err = f.WriteString(w.all())
	w.builder.Reset()
	return err
}

func walkWrite(deep int, n *node) error {
	w.writeHeadline(deep, n.Data.Title)
	if n.Data.Link != "" {
		if n.Data.LinkTitle != "" {
			w.writeLink(n.Data.LinkTitle, n.Data.Link)
		} else {
			w.writeLink("详见此文", n.Data.Link)
		}
	}
	if len(n.Data.Note) == 0 {
		w.writeBlankLine()
	} else {
		w.writeContent(n.Data.Note)
	}
	for _, c := range n.Children {
		if err := walkWrite(deep+1, c); err != nil {
			return err
		}
	}
	return nil
}

type writer struct {
	builder *strings.Builder
}

func (w *writer) writeHeadline(n int, headline string) error {
	if n > 7 {
		n = 7
	}
	w.builder.Grow(n + 1 + len(headline) + 2)
	for n > 0 {
		w.builder.WriteByte('#')
		n--
	}
	w.builder.WriteByte(' ')
	_, err := w.builder.WriteString(headline)
	w.builder.WriteByte('\r')
	w.builder.WriteByte('\n')
	return err
}

func (w *writer) writeBlankLine() {
	w.builder.Grow(2)
	w.builder.WriteByte('\r')
	w.builder.WriteByte('\n')
}

func (w *writer) writeLink(title, link string) {
	w.builder.Grow(8 + len(title) + len(link))
	w.builder.WriteByte('\r')
	w.builder.WriteByte('\n')
	w.builder.WriteByte('[')
	w.builder.WriteString(title)
	w.builder.WriteByte(']')
	w.builder.WriteByte('(')
	w.builder.WriteString(link)
	w.builder.WriteByte(')')
	w.builder.WriteByte('\r')
	w.builder.WriteByte('\n')
}

func (w *writer) writeContent(content string) error {
	w.builder.Grow(len(content) + 4)
	w.builder.WriteByte('\r')
	w.builder.WriteByte('\n')
	_, err := w.builder.WriteString(content)
	w.builder.WriteByte('\r')
	w.builder.WriteByte('\n')
	return err
}

func (w *writer) all() string {
	return w.builder.String()
}

type data struct {
	Id        string `json:"id"`
	Ts        int64  `json:"created"`
	Title     string `json:"text"`
	Note      string `json:"note"`
	LinkTitle string `json:"hyperlinkTitle"`
	Link      string `json:"hyperlink"`
}

type node struct {
	Data     data    `json:"data"`
	Children []*node `json:"children"`
}
```