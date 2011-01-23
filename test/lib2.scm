(define-module (test lib2)
  :use-module (srfi srfi-1)
  :use-module (test lib)
  :export (curses-test-start
	   curses-test-end))

(define (curses-test-start)
  (let ((count-reporter (make-count-reporter))
	(out-string (open-output-string)))
    (register-reporter (make-log-reporter out-string))
    (register-reporter (first count-reporter))
    (list (second count-reporter)
	  out-string)))

;; Exit the test.  If an error was detected, save an error
;; log into FILENAME.
(define (curses-test-end reporter-and-string filename)
  (let* ((count-reporter (first reporter-and-string))
	 (out-string     (second reporter-and-string))
	 (results        (count-reporter)))
    (for-each
     (lambda (tag)
       (let ((count (assq-ref results tag)))
	 (if (> count 0)
	     (let ((port (open-output-file filename)))
	       (display (get-output-string out-string) port)
	       (display (get-output-string out-string))
	       (exit 1)))))
     '(fail upass error))
    (exit 0)))
