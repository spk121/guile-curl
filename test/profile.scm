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

(use-modules (curl)
             (rnrs bytevectors))

(define bv-1k (make-bytevector 1024))
(define bv-10k (make-bytevector (* 10 1024)))
(define bv-100k (make-bytevector (* 100 1024)))

(define (init-bytevector! bv)
  (let ((len (bytevector-length bv)))
    (let loop ((i 0))
      (when (< i len)
        (bytevector-u8-set! bv i (modulo i 256))
        (loop (1+ i))))))

(init-bytevector! bv-1k)
(init-bytevector! bv-10k)
(init-bytevector! bv-100k)

; (write bv-1k) (newline)

(define (with-timer name thunk)
  (format #t "Test: ~A~%" name)
  (let* ((start-time (times))
         (ret (thunk))
         (end-time (times)))
    ;; (write (utf8->string ret)) (newline)
    (format #t "clock: ~A, utime: ~A, cutime ~A, cstime ~A~%"
            (- (tms:clock end-time) (tms:clock start-time))
            (- (tms:utime end-time) (tms:utime start-time))
            (- (tms:cutime end-time) (tms:cutime start-time))
            (- (tms:cstime end-time) (tms:cstime start-time)))))

(define (test-bv-thunk bv N)
  (lambda ()
    (let loop ((i 0)
               (tmp #f))
      (if (< i N)
          (loop
           (1+ i)
           (bv->url-encoded-bv2 bv))
          tmp))))

(with-timer "short bytevector"
            (test-bv-thunk bv-1k 1000))

(with-timer "medium bytevector"
            (test-bv-thunk bv-10k 100))

(with-timer "long bytevector"
            (test-bv-thunk bv-100k 10))
