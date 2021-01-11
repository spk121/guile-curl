;; -*- Mode: scheme; -*-

;;; Copyright (C) 2021 Michael L. Gran

;;; This program is free software: you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License as published by
;;; the Free Software Foundation, either version 3 of the License, or
;;; (at your option) any later version.

;;; This program is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.

;;; You should have received a copy of the GNU General Public License
;;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

(use-modules (curl) (srfi srfi-64) (rnrs bytevectors))

(test-begin "bytevectors")

(define (string->latin1-bytevector str)
  (u8-list->bytevector (map char->integer (string->list str))))

(define (latin1-bytevector->string bv)
  (apply string (map integer->char (bytevector->u8-list bv))))

(test-equal "ascii text is unchanged by bv->url-encoded-bv"
  "hello"
  (latin1-bytevector->string
   (bv->url-encoded-bv
    (string->latin1-bytevector "hello"))))

(test-equal "whitespace text is modified by bv->url-encoded-bv"
  "%20%09%0D%0A"
  (latin1-bytevector->string
   (bv->url-encoded-bv
    (string->latin1-bytevector
     (string #\space #\tab #\return #\newline)))))

(test-end "bytevectors")
