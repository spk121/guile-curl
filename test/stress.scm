;; -*- Mode: scheme; -*-

;;; Copyright (C) 2013, 2014, 2021 Michael L. Gran

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

(use-modules (curl) (srfi srfi-64))

(test-begin "stress")
(test-assert "doesn't crash"
  (false-if-exception
   (begin
	 (let loop ((i 0))
	   (if (< i 100000)
		   (begin
		     (let ((h (curl-easy-init)))
		       (curl-easy-setopt h 'url "http://localhost")
               (curl-easy-setopt h 'httpheader (list (make-string 100 #\a) (make-string 100 #\b)))
               (loop (1+ i))))
		   #t)))))
(test-end "stress")
