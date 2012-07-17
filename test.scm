(define (factorial n)
  (define (iter product counter max-count)
    (if (> counter max-count)
      product
      (iter (+ counter product)
            (+ counter 1)
            max-count)))
  (iter 1 1 n))

"(factorial 5):"
(factorial 5)

(define (fact x)
  (if (= x 0)
    1
    (* x (fact (- x 1)))))
(fact 5)

(define (range-recur cur target)
  (if (= cur target)
    (list target)
    (cons cur (range-recur (+ cur 1) target))))

(define (range-r x)
  (range-recur 0 x))

(range-r 10)

(define (dec x)
  (- x 1))
(define (inc x)
  (+ x 1))

(define (range end)
  (let ((cur (dec end))
        (accum '()))
    (while (>= cur 0)
      (set! accum (cons cur accum))
      (set! cur (dec cur)))
    accum))

(range 10)
